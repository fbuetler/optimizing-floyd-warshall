import argparse
import math
import numpy as np
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
parser.add_argument("-n", "--input-size", help="input size", type=int, required=True)
# parser.add_argument("-n", "--min-n", help="minimum input size", type=int, required=True)
# parser.add_argument("-N", "--max-n", help="maximum input size", type=int, required=True)
parser.add_argument(
    "-vec",
    "--vectorize",
    help="generated code should use vector instructions",
    action="store_true",
)

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


def generate_fw(
    inpath,
    outpath,
    algorithm,
    implementation,
    compiler,
    opt_flags_raw,
    unroll_tile_list,
):
    opt_flags = opt_flags_raw.replace(" ", "_")

    generated_files = list()
    for ui, uj, ti, tj in unroll_tile_list:
        logging.info(f"generating code: unroll ({ui}, {uj}), tile ({ti}, {tj})")
        output_fname = f"{outpath}/{algorithm}_{implementation}-ui{ui}-uj{uj}-ti{ti}-tj{tj}_{compiler}_{opt_flags}.c"

        context = dict()
        context["unroll_i"] = ui
        context["unroll_j"] = uj
        context["tilesizei"] = ti
        context["tilesizej"] = tj

        with open(output_fname, mode="w", encoding="utf-8") as f:
            f.write(
                render_jinja_template(
                    f"{inpath}",
                    f"fw-unroll-tile.py.j2",
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


def validate_fw(
    project_root, algorithm, implementation, compiler, opt_flags, unroll_tile_list
):
    for ui, uj, ti, tj in unroll_tile_list:
        logging.info(f"validating code: unroll ({ui}, {uj}), tile ({ti}, {tj})")

        testcases = list()
        for n in [4, 8, 16, 30, 32]:
            if n % ti == 0 and n % tj == 0:
                testcases.append(f"n{n}")

        if len(testcases) == 0:
            # TODO
            logging.warning("Skipping validation as there are no fiting testcases")
            continue

        validate_cmd = [
            "bash",
            f"{project_root}/team7.sh",
            "validate",
            f"{algorithm}",
            f"{implementation}-ui{ui}-uj{uj}-ti{ti}-tj{tj}",
            f"{compiler}",
            f"{opt_flags}",
            f"{','.join(testcases)}",
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


def measure_fw(
    project_root,
    algorithm,
    implementation,
    compiler,
    opt_flags,
    test_input,
    input_size,
    unroll_tile_list,
):
    for ui, uj, ti, tj in unroll_tile_list:
        logging.info(f"measuring code: unroll ({ui}, {uj}), tile ({ti}, {tj})")
        measure_cmd = [
            "bash",
            f"{project_root}/team7.sh",
            "measure",
            f"{algorithm}",
            f"{implementation}-ui{ui}-uj{uj}-ti{ti}-tj{tj}",
            f"{compiler}",
            f"{opt_flags}",
            f"{test_input}",
            f"n{input_size}",
        ]

        logging.debug(" ".join(measure_cmd))
        result = subprocess.run(measure_cmd, capture_output=True, text=True)
        logging.debug(result.stdout)
        if result.returncode != 0:
            logging.error(result.stderr)
            return result.returncode

    return 0


def get_optimal_perf(
    data_root,
    algorithm,
    implementation,
    compiler,
    opt_flags_raw,
    test_input,
    input_size,
    unroll_tile_list,
):
    opt_flags = opt_flags_raw.replace(" ", "_")

    ui_opt = 0
    uj_opt = 0
    ti_opt = 0
    tj_opt = 0
    p_opt = 0

    for ui, uj, ti, tj in unroll_tile_list:
        logging.info(f"processing data: unroll ({ui}, {uj}), tile ({ti}, {tj})")
        data_fname = f"{data_root}/{algorithm}_{implementation}-ui{ui}-uj{uj}-ti{ti}-tj{tj}_{compiler}_{opt_flags}_{test_input}.csv"
        with open(data_fname) as f:
            reader = csv.reader(f, delimiter=",", quoting=csv.QUOTE_NONNUMERIC)
            n_list = reader.__next__()
            runs_list = reader.__next__()
            cycles_list = reader.__next__()

            n_ref = -1
            for i, n in enumerate(n_list):
                if n == input_size:
                    n_ref = i
                    break

            if n_ref == -1:
                logging.error("required column not found")
                raise Exception("failed to get optimal perf")

            c_ref = cycles_list[n_ref]
            p_ref = round((2 * n * n * n) / c_ref, 2)

            logging.debug(
                f"performance for unroll ({ui}, {uj}), tile ({ti}, {tj})= {p_ref}"
            )

            if p_ref > p_opt:
                p_opt = p_ref
                ui_opt = ui
                uj_opt = uj
                ti_opt = ti
                tj_opt = tj

    return (ui_opt, uj_opt, ti_opt, tj_opt, p_opt)


def get_best_perf(project_root, input_size, input, unroll_tile_list):

    clean_files(project_root)

    generated_files = generate_fw(
        f"{project_root}/autotuning",
        f"{project_root}/autotuning/generated/shortest-path",
        ALGORITHM,
        IMPLEMENTATION,
        COMPILER,
        OPT_FLAGS,
        unroll_tile_list,
    )
    if len(generated_files) == 0:
        raise Exception("Generate files failed")

    # build all generated files
    retcode = build_files(project_root, ALGORITHM, IMPLEMENTATION, COMPILER, OPT_FLAGS)
    if retcode != 0:
        raise Exception("Build files failed")

    # validate all built files
    retcode = validate_fw(
        project_root, ALGORITHM, IMPLEMENTATION, COMPILER, OPT_FLAGS, unroll_tile_list
    )
    if retcode != 0:
        raise Exception("Validate files failed")

    # measure all validates files
    retcode = measure_fw(
        project_root,
        ALGORITHM,
        IMPLEMENTATION,
        COMPILER,
        OPT_FLAGS,
        input,
        input_size,
        unroll_tile_list,
    )
    if retcode != 0:
        raise Exception("Measure files failed")

    # use measurements as feedback
    opt_ui, opt_uj, opt_ti, opt_tj, p_opt = get_optimal_perf(
        f"{project_root}/measurements/data",
        ALGORITHM,
        IMPLEMENTATION,
        COMPILER,
        OPT_FLAGS,
        input,
        input_size,
        unroll_tile_list,
    )
    logging.info(
        f"optimal performance with:\nunrollment: ({opt_ui}, {opt_uj})\ntile: ({opt_ti}, {opt_tj})\nperformance: {p_opt}"
    )

    return (opt_ui, opt_uj, opt_ti, opt_tj)


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

    unroll_tile_list = list()
    for i in range(min_ui, max_ui + 1):
        for j in range(min_uj, max_uj + 1):
            unroll_tile_list.append((i, j, "N", "N"))

    ui, uj, ti, tj = get_best_perf(
        project_root,
        64 if not is_debug_run else 32,
        BENCH_INPUT if not is_debug_run else TEST_INPUT,
        unroll_tile_list,
    )

    return ui, uj


def unrollment_hill_climbing(project_root, ui, uj, is_debug_run=False):
    while True:
        logging.info(f"climing hill around unrollment ({ui}, {uj})")

        unroll_tile_list = list()
        for i in range(ui - 1, ui + 2):
            for j in range(uj - 1, uj + 2):
                if i == ui or j == uj:
                    # skip the rock we are standing on
                    continue
                if i < 1 or j < 1:
                    # skip unrollment factors that make no sense
                    continue
                unroll_tile_list.append((i, j, "N", "N"))

        next_ui, next_uj, next_ti, next_tj = get_best_perf(
            project_root,
            64 if not is_debug_run else 32,
            BENCH_INPUT if not is_debug_run else TEST_INPUT,
            unroll_tile_list,
        )
        if next_ui == ui and next_uj == uj:
            break
        ui = next_ui
        uj = next_uj

    logging.info(f"reached top ({ui}, {uj})")
    return ui, uj


def tile_l2_hill_climbing(
    project_root, input_size, l2_cache_bytes, ui, uj, is_debug_run=False
):
    t2 = 2 ** math.floor(np.log2(math.sqrt(l2_cache_bytes / 3)))  # TODO

    if is_debug_run:
        t2 = 4

    if t2 == 0:
        raise Exception("heuristic makes no sense")

    visited = set()
    while True:
        logging.info(f"climing hill around unrollement ({ui}, {uj}), tile ({t2})")
        unroll_tile_list = list()
        for i in range(ui - 1, ui + 2):
            for j in range(uj - 1, uj + 2):
                if i == ui or j == uj:
                    # skip the rock we are standing on
                    continue
                if i < 1 or j < 1:
                    # skip unrollment factors that make no sense
                    continue

                for t in [t2 / 2, t2 * 2]:
                    t = int(t)
                    if t < 1:
                        # skip unrollment factors that make no sense
                        logging.debug(
                            "skipping rock: unrollment factor would make no sense"
                        )
                        continue

                    if i > t or j > t:
                        # tile size should ALWAYS be larger than the unrolling factor
                        logging.debug(
                            "skipping rock: tile size should be larger than unrolling factor"
                        )
                        continue

                    if t > input_size:
                        # tile size cannot be large than the testcase size
                        logging.debug(
                            "skipping rock: tile size cannot be larger than the testcase"
                        )
                        continue

                    # dont climb the same rock twice
                    rock = f"ui{i}_uj{j}_ti{t}_tj{t}"
                    if rock in visited:
                        continue

                    visited.add(rock)

                    # we use only squared tiles here
                    unroll_tile_list.append((i, j, t, t))

        if len(unroll_tile_list) == 0:
            logging.info(f"all visited:\n{visited}")
            raise Exception("no place to go")

        next_ui, next_uj, next_ti, next_tj = get_best_perf(
            project_root,
            input_size,
            BENCH_INPUT if not is_debug_run else TEST_INPUT,
            unroll_tile_list,
        )
        if next_ui == ui and next_uj == uj and next_ti == t2:
            break
        ui = next_ui
        uj = next_uj
        t2 = next_ti

    logging.info(f"reached top with  unrollment ({ui}, {uj}), tile ({t2}, {t2})")
    return ui, uj


def main(project_root, input_size, l1_cache_bytes, l2_cache_bytes, vectorize):
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
        project_root, initial_ui, initial_uj, is_debug_run=debug
    )

    # refined_ui = 8
    # refined_uj = 1
    tile_l2_hill_climbing(
        project_root,
        input_size,
        l2_cache_bytes,
        refined_ui,
        refined_uj,
        is_debug_run=debug,
    )


if __name__ == "__main__":
    args = parser.parse_args()
    main(
        args.project_root,
        args.input_size,
        args.l1_cache,
        args.l2_cache,
        args.vectorize,
    )
