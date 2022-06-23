import argparse
import csv
import logging
import pathlib
from jinja2 import pass_eval_context

import matplotlib as mpl
import matplotlib.pyplot as plt

logging.basicConfig(level=logging.INFO)

SMALL_SIZE = 13
MEDIUM_SIZE = 15
BIGGER_SIZE = 18

plt.rc('font', size=MEDIUM_SIZE)          # controls default text sizes
plt.rc('axes', titlesize=MEDIUM_SIZE)     # fontsize of the axes title
plt.rc('axes', labelsize=MEDIUM_SIZE)    # fontsize of the x and y labels
plt.rc('xtick', labelsize=SMALL_SIZE)    # fontsize of the tick labels
plt.rc('ytick', labelsize=SMALL_SIZE)    # fontsize of the tick labels
plt.rc('legend', fontsize=MEDIUM_SIZE)    # legend fontsize
plt.rc('figure', titlesize=BIGGER_SIZE)  # fontsize of the figure title

COLOR_LIST = [
    "#1b9e77",
    "#d95f02",
    "#7570b3",
    "#e7298a",
    "#66a61e",
    "#e6ab02",
    "#a6761d",
    "#666666",
    "#332288",
    "#88CCEE",
    "#44AA99",
    "#117733",
    "#999933",
    "#DDCC77",
    "#CC6677",
    "#882255",
    "#AA4499",
    "#f58231",
    "#0082C8",
    "#F58231",
    "#911EB4",
    "#008080",
    "#AA6E28",
    "#800000",
    "#808000",
]

LABEL_ALGORITHM = "algo"
LABEL_IMPLEMENTATION = "impl"
LABEL_COMPILER = "comp"
LABEL_OPTS = "opts"
LABEL_TESTSUITE = "testsuite"
LABEL_CHOICES = [
    LABEL_ALGORITHM,
    LABEL_IMPLEMENTATION,
    LABEL_COMPILER,
    LABEL_OPTS,
    LABEL_TESTSUITE,
]

parser = argparse.ArgumentParser(
    description="Generate performance plot using data from one or more csv files. One line is added for each csv file."
)
parser.add_argument(
    "-d", "--data", help="list of csv data files", type=str, nargs="+", required=True
)
parser.add_argument(
    "-l",
    "--labels",
    help="list of labels to use for plot data",
    choices=LABEL_CHOICES,
    nargs="+",
    required=True,
)
parser.add_argument(
    "-p", "--plot", help="directory to save the generated plot", type=str, required=True
)
parser.add_argument("-t", "--title", help="title for the plot", type=str, required=True)
parser.add_argument(
    "-pi",
    "--peak",
    help="maximum achievable performance in flops/cycle",
    type=float,
    default=4.0,
)
parser.add_argument(
    "-vpi",
    "--simd-peak",
    help="maximum achievable performance in flops/cycle using SIMD instructions",
    type=float,
    default=16.0,
)
parser.add_argument(
    "-bp",
    "--bit-packed",
    help="implementation of plot is bit-packed",
    action="store_true",
)
parser.add_argument("-o", "--output", help="output file name", type=str, required=True)


def main(
    data_file_list,
    label_choices,
    plots_dir,
    title,
    peak,
    peak_simd,
    bit_packed,
    output_file,
):
    mpl.rcParams["axes.prop_cycle"] = mpl.cycler(
        color=COLOR_LIST
    )  # sets colors using given cycle

    mpl.rcParams["figure.figsize"] = [8, 5]

    logging.info("generating performance plot...")

    # set up bound visualizations
    _, ax = plt.subplots()

    # plot performance bounds
    pi_color = next(ax._get_lines.prop_cycler)["color"]
    piv_color = next(ax._get_lines.prop_cycler)["color"]
    plt.axhline(
        y=peak,  linewidth=1, color=pi_color
    )
    plt.axhline(
        y=peak_simd,
        linewidth=1,
        color=piv_color,
    )
    ax.annotate('P ≤ π', xy=(1.01,peak), xytext=(0.0, -11), xycoords=('axes fraction', 'data'), textcoords='offset points', color=pi_color)
    ax.annotate('P ≤ π-SIMD', xy=(1.01,peak_simd), xytext=(0.0, 0), xycoords=('axes fraction', 'data'), textcoords='offset points', color=piv_color)

    naive_max = 0.0
    perf_max = 0.0
    perf_max_scalar = 0.0
    perf_max_simd = 0.0
    for data_file_path in data_file_list:

        # generate label
        data_file = pathlib.Path(data_file_path).stem
        parts = data_file.split("_")

        algorithm = parts[0]
        implementation = parts[1]
        compiler = parts[2]
        opts = " ".join(parts[3:-1])
        testsuite = parts[-1]

        label_list = list()
        if LABEL_ALGORITHM in label_choices:
            label_list.append(algorithm)
        if LABEL_IMPLEMENTATION in label_choices:
            label_list.append(implementation)
        if LABEL_COMPILER in label_choices:
            label_list.append(compiler)
        if LABEL_OPTS in label_choices:
            label_list.append(opts)
        if LABEL_TESTSUITE in label_choices:
            label_list.append(testsuite)
        label = " ".join(label_list)

        # read cycles
        with open(data_file_path) as f:
            reader = csv.reader(f, delimiter=",", quoting=csv.QUOTE_NONNUMERIC)
            n_list = reader.__next__()
            runs_list = reader.__next__()
            cycles_list = reader.__next__()

        # compute performance (flops = 2 * n^3)
        perf_list = list()
        for (n, c, r) in zip(n_list, cycles_list, runs_list):
            perf_list.append((2 * n * n * n) / c)

        plt.plot(n_list, perf_list, label=label, marker="o")

        if implementation == 'naive':
            naive_max = max(perf_list + [naive_max])
        else:
            perf_max = max(perf_list + [perf_max])

        if 'SIMD' in implementation:
            perf_max_simd = max(perf_list + [perf_max_simd])
        else:
            perf_max_scalar = max(perf_list + [perf_max_scalar])

    logging.info(f"For {algorithm}: Maximum speedup is {perf_max/naive_max}")
    logging.info(f"For {algorithm}: Maximum % of peak for SIMD is {perf_max_simd / peak_simd * 100}%")
    logging.info(f"For {algorithm}: Maximum % of peak for scalar is {perf_max_scalar / peak * 100}%")



    # configure plot
    plt.ylabel("P(n) [flops/cycle]")
    plt.xlabel("n")
    plt.xscale("log", base=2)
    plt.yscale("log", base=2)
    plt.grid(True, which="major", axis="y")
    plt.title(title)

    # reorder legend
    handles, labels = plt.gca().get_legend_handles_labels()
    labels, handles = zip(
        *sorted(zip(labels, handles), key=lambda t: t[1]._y[0], reverse=True)
    )
    plt.legend(handles, labels, loc="lower left", bbox_to_anchor=(1, 0.48))
    plt.tight_layout()

    logging.info("storing performance plot to {}".format(output_file))
    outfile = "{}/{}_perf.eps".format(plots_dir, output_file)
    plt.savefig(outfile)
    plt.clf()



if __name__ == "__main__":
    args = parser.parse_args()
    main(
        args.data,
        args.labels,
        args.plot,
        args.title,
        args.peak,
        args.simd_peak,
        args.bit_packed,
        args.output,
    )
