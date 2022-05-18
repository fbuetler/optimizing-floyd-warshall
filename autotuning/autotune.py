import argparse
import math
import jinja2
import subprocess
import logging
import csv

logging.basicConfig(encoding="utf-8", level=logging.INFO)

parser = argparse.ArgumentParser()
parser.add_argument(
    "-p", "--project-root", help="path of the project root", type=str, required=True
)
parser.add_argument(
    "-l1", "--l1-cache", help="size of the L1 cache in bytes", type=int, required=True
)
parser.add_argument(
    "-l2", "--l2-cache", help="size of the L2 cache in bytes", type=int, required=True
)
parser.add_argument("-n", "--min-n", help="minimum input size", type=int, required=True)
parser.add_argument("-N", "--max-n", help="maximum input size", type=int, required=True)
parser.add_argument(
    "-vec",
    "--vectorize",
    help="generated code should use vector instructions",
    action="store_true",
)
parser.add_argument("-o", "--output", help="output file path", type=str, required=True)

ALGORITHM = "fw"
IMPLEMENTATION = "c-autotune"
COMPILER = "gcc"
OPT_FLAGS = "-O3 -fno-tree-vectorize"
TEST_INPUT = "test-inputs"
BENCH_INPUT = "bench-inputs"


def clean_files(project_root):
    clean_cmd = [
        "bash",
        f"{project_root}/team7.sh",
        "clean",
    ]

    logging.debug(" ".join(clean_cmd))
    result = subprocess.run(clean_cmd)
    return result.returncode


def render_jinja_template(template_loc, file_name, **context):
    return (
        jinja2.Environment(loader=jinja2.FileSystemLoader(template_loc + "/"))
        .get_template(file_name)
        .render(context)
    )


def generate_fw_unroll(
    inpath,
    outpath,
    algorithm,
    implementation,
    compiler,
    opt_flags_raw,
    min_ui,
    max_ui,
    min_uj,
    max_uj,
):
    opt_flags = opt_flags_raw.replace(" ", "_")

    generated_files = list()
    for ui in range(min_ui, max_ui + 1):
        for uj in range(min_uj, max_uj + 1):
            logging.info("generating unrolled code: ui = {}, uj = {}".format(ui, uj))
            output_fname = f"{outpath}/{algorithm}_{implementation}-unroll-ui{ui}-uj{uj}_{compiler}_{opt_flags}.c"

            context = dict()
            context["unroll_i"] = ui
            context["unroll_j"] = uj

            with open(output_fname, mode="w", encoding="utf-8") as f:
                f.write(
                    render_jinja_template(
                        f"{inpath}",
                        f"fw-unroll.py.j2",
                        **context,
                    )
                )
            generated_files.append(output_fname)

    return generated_files


def build_files(project_root, algorithm, implementation, compiler, opt_flags_raw):
    build_cmd = [
        "make",
        "-C",
        f"{project_root}",
        f"build-{algorithm}-{implementation}-{compiler}",
        "-e",
        f"CFLAGS_DOCKER={opt_flags_raw}",
    ]

    logging.debug(" ".join(build_cmd))
    result = subprocess.run(build_cmd)
    return result.returncode


def validate_fw_unroll(
    project_root,
    algorithm,
    implementation,
    compiler,
    opt_flags,
    min_ui,
    max_ui,
    min_uj,
    max_uj,
):
    for ui in range(min_ui, max_ui + 1):
        for uj in range(min_uj, max_uj + 1):
            logging.info("validating code: ui = {}, uj = {}".format(ui, uj))
            validate_cmd = [
                "bash",
                f"{project_root}/team7.sh",
                "validate",
                f"{algorithm}",
                f"{implementation}-unroll-ui{ui}-uj{uj}",
                f"{compiler}",
                f"{opt_flags}",
            ]

            logging.debug(" ".join(validate_cmd))
            result = subprocess.run(
                validate_cmd,
                text=True,
                capture_output=True,
            )
            logging.debug(result.stdout)
            if result.returncode != 0:
                logging.error(result.stderr)
                return result.returncode

    return 0


def measure_fw_unroll(
    project_root,
    algorithm,
    implementation,
    compiler,
    opt_flags,
    test_input,
    min_ui,
    max_ui,
    min_uj,
    max_uj,
):
    for ui in range(min_ui, max_ui + 1):
        for uj in range(min_uj, max_uj + 1):
            logging.info("measuring code: ui = {}, uj = {}".format(ui, uj))
            measure_cmd = [
                "bash",
                f"{project_root}/team7.sh",
                "measure",
                f"{algorithm}",
                f"{implementation}-unroll-ui{ui}-uj{uj}",
                f"{compiler}",
                f"{opt_flags}",
                f"{test_input}",
            ]

            logging.debug(" ".join(measure_cmd))
            result = subprocess.run(measure_cmd, capture_output=True, text=True)
            logging.debug(result.stdout)
            if result.returncode != 0:
                logging.error(result.stderr)
                return result.returncode

    return 0


def get_optimal_unrollment(
    data_root,
    algorithm,
    implementation,
    compiler,
    opt_flags_raw,
    test_input,
    min_ui,
    max_ui,
    min_uj,
    max_uj,
):
    opt_flags = opt_flags_raw.replace(" ", "_")

    ui_opt = 0
    uj_opt = 0
    p_opt = 0

    for ui in range(min_ui, max_ui + 1):
        for uj in range(min_uj, max_uj + 1):
            logging.info("processing data: ui = {}, uj = {}".format(ui, uj))
            data_fname = f"{data_root}/{algorithm}_{implementation}-unroll-ui{ui}-uj{uj}_{compiler}_{opt_flags}_{test_input}.csv"
            with open(data_fname) as f:
                reader = csv.reader(f, delimiter=",", quoting=csv.QUOTE_NONNUMERIC)
                n_list = reader.__next__()
                runs_list = reader.__next__()
                cycles_list = reader.__next__()

                n_64 = -1
                for i, n in enumerate(n_list):
                    if n == 32.0:
                        # if n == 64: TODO
                        n_64 = i
                        break

                if n_64 == -1:
                    logging.error("required column not found")
                    raise Exception("failed to get optimal unrollment factor")

                c_64 = cycles_list[n_64]
                p_64 = round((2 * n * n * n) / c_64, 2)

                logging.debug(f"performance for ({ui}, {uj}) = {p_64}")

                if p_64 > p_opt:
                    p_opt = p_64
                    ui_opt = ui
                    uj_opt = uj

    return (ui_opt, uj_opt, p_opt)


def get_best_unrollment(project_root, input, min_ui, max_ui, min_uj, max_uj):

    clean_files(project_root)

    generated_unnoll_files = generate_fw_unroll(
        f"{project_root}/autotuning",
        f"{project_root}/autotuning/generated/shortest-path",
        ALGORITHM,
        IMPLEMENTATION,
        COMPILER,
        OPT_FLAGS,
        min_ui,
        max_ui,
        min_uj,
        max_uj,
    )
    if len(generated_unnoll_files) == 0:
        logging.error("Generate files failed")
        return 1

    # build all generated files
    retcode = build_files(project_root, ALGORITHM, IMPLEMENTATION, COMPILER, OPT_FLAGS)
    if retcode != 0:
        logging.error("Build files failed")
        return retcode

    # validate all built files
    retcode = validate_fw_unroll(
        project_root,
        ALGORITHM,
        IMPLEMENTATION,
        COMPILER,
        OPT_FLAGS,
        min_ui,
        max_ui,
        min_uj,
        max_uj,
    )
    if retcode != 0:
        logging.error("Validate files failed")
        return retcode

    # measure all validates files
    retcode = measure_fw_unroll(
        project_root,
        ALGORITHM,
        IMPLEMENTATION,
        COMPILER,
        OPT_FLAGS,
        input,
        min_ui,
        max_ui,
        min_uj,
        max_uj,
    )
    if retcode != 0:
        logging.error("Measure files failed")
        return retcode

    # use measurements as feedback
    opt_ui, opt_uj, p_opt = get_optimal_unrollment(
        f"{project_root}/measurements/data",
        ALGORITHM,
        IMPLEMENTATION,
        COMPILER,
        OPT_FLAGS,
        input,
        min_ui,
        max_ui,
        min_uj,
        max_uj,
    )
    logging.info(f"optimal unrollment is: ({opt_ui}, {opt_uj}) with {p_opt}")

    return opt_ui, opt_uj


def unrollment_initial_guess(project_root, is_debug_run=False):
    min_ui = 1
    max_ui = 16
    min_uj = 1
    max_uj = 32

    if is_debug_run:
        min_ui = 4
        max_ui = 6
        min_uj = 4
        max_uj = 6

    ui, uj = get_best_unrollment(
        project_root, TEST_INPUT, min_ui, max_ui, min_uj, max_uj
    )

    return ui, uj


def unrollment_hill_climbing(project_root, ui, uj):
    while True:
        logging.info(f"climing hill around ({ui}, {uj})")
        # TODO can be optimized by not running (ui, uj)
        next_ui, next_uj = get_best_unrollment(
            project_root, BENCH_INPUT, max(ui - 1, 1), ui + 1, max(uj - 1, 1), uj + 1
        )
        if next_ui == ui and next_uj == uj:
            break
        ui = next_ui
        uj = next_uj

    logging.info(f"reached top ({ui}, {uj})")
    return ui, uj


def tile_l2_hill_climbing(project_root, l2_cache_bytes):
    l2 = math.floor(math.sqrt(l2_cache_bytes / 3))
    # TODO


def main(
    project_root, l1_cache_bytes, l2_cache_bytes, min_n, max_n, vectorize, output_fname
):
    # debug = True
    debug = False

    if debug:
        logging.basicConfig(encoding="utf-8", level=logging.DEBUG, force=True)

    # exhaustive search with test input
    initial_ui, initial_uj = unrollment_initial_guess(project_root, is_debug_run=debug)

    # hill climbing search with bench input
    # initial_ui = 9
    # initial_uj = 1
    refined_ui, refined_uj = unrollment_hill_climbing(
        project_root, initial_ui, initial_uj
    )

    # refined_ui = 8
    # refined_uj = 1
    tile_l2_hill_climbing(project_root, l2_cache_bytes)


if __name__ == "__main__":
    args = parser.parse_args()
    main(
        args.project_root,
        args.l1_cache,
        args.l2_cache,
        args.min_n,
        args.max_n,
        args.vectorize,
        args.output,
    )
