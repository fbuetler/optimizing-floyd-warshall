import argparse
import os
import sys
import jinja2

parser = argparse.ArgumentParser()
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


def autotune_fw_unroll(path, min_ui, max_ui, min_uj, max_uj):

    for ui in range(min_ui, max_ui + 1):
        for uj in range(min_uj, max_uj + 1):
            print("generating unrolled code: ui = {}, uj = {}".format(ui, uj))
            output_fname = f"{path}/fw-c-autotune-unroll-ui{ui}_uj{uj}.c"

            context = dict()
            context["unroll_i"] = ui
            context["unroll_j"] = uj

            with open(output_fname, mode="w", encoding="utf-8") as f:
                f.write(
                    render_jinja_template(
                        "./",
                        f"fw-unroll.py.j2",
                        **context,
                    )
                )


def main(l1_cache_bytes, l2_cache_bytes, min_n, max_n, vectorize, output_fname):

    autotune_fw_unroll("generated", 1, 16, 1, 32)


if __name__ == "__main__":
    args = parser.parse_args()
    main(
        args.l1_cache,
        args.l2_cache,
        args.min_n,
        args.max_n,
        args.vectorize,
        args.output,
    )
