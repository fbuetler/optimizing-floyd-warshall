import argparse
import jinja2
import subprocess

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


def render_jinja_template(template_loc, file_name, **context):
    return (
        jinja2.Environment(loader=jinja2.FileSystemLoader(template_loc + "/"))
        .get_template(file_name)
        .render(context)
    )


def generate_fw_unroll(
    inpath, outpath, algorithm, implementation, compiler, min_ui, max_ui, min_uj, max_uj
):

    generated_files = list()
    for ui in range(min_ui, max_ui + 1):
        for uj in range(min_uj, max_uj + 1):
            print("generating unrolled code: ui = {}, uj = {}".format(ui, uj))
            output_fname = f"{outpath}/{algorithm}_{implementation}-unroll-ui{ui}-uj{uj}_{compiler}.c"

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


def build_files(project_root, algorithm, implementation, compiler, opt_flags):
    build_cmd = [
        "make",
        "-C",
        f"{project_root}",
        f"build-{algorithm}-{implementation}-{compiler}",
        "-e",
        f"CFLAGS_DOCKER={opt_flags}",
    ]

    print(build_cmd)
    subprocess.call(build_cmd)


def main(
    project_root, l1_cache_bytes, l2_cache_bytes, min_n, max_n, vectorize, output_fname
):
    algorithm = "fw"
    implementation = "c-autotune"
    compiler = "gcc"
    opt_flags = "-O3 -fno-tree-vectorize"

    # generate
    # min_ui = 1
    # max_ui = 16
    # min_uj = 1
    # max_uj = 32
    min_ui = 4
    max_ui = 6
    min_uj = 4
    max_uj = 6
    generated_unnoll_files = generate_fw_unroll(
        f"{project_root}/autotuning",
        f"{project_root}/autotuning/generated/shortest-path",
        algorithm,
        implementation,
        compiler,
        min_ui,
        max_ui,
        min_uj,
        max_uj,
    )

    # build all generated files
    build_files(project_root, algorithm, implementation, compiler, opt_flags)

    # validate all built files
    # validate_files()

    # measure all validates files

    # use measurments as feedback


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
