import argparse
import csv
import logging
import pathlib
from typing import Dict, List, Tuple
import matplotlib.pyplot as plt
import matplotlib as mpl
from cycler import cycler

logging.basicConfig(level=logging.INFO)

USE_Q_ESTIMATE = False

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
    "-b",
    "--beta",
    help="memory bandwidth in bytes/cycle",
    type=float,
    default=2.0,
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


def estimate_q(n: int, datasize: int, l3_total_size: int, l3_cache_line: int) -> int:
    """computes total number of bytes transfered from memory to L3 cache"""
    if n**2 * datasize < l3_total_size:
        # only cold misses
        return n**2 * l3_cache_line
    else:
        # assume n >> cache size
        op1_misses = n**2
        op2_misses = n**2 * (n * datasize // l3_cache_line) - n
        op3_misses = n**2 * (n * datasize // l3_cache_line) - 2 * n**2
        return (op1_misses + op2_misses + op3_misses) * l3_cache_line


def roofline_plot(
    data_file_list: List[str],
    pi: float,
    pi_simd: float,
    beta: float,
    bit_packed: bool,
    label_choices: List[str],
    plots_dir: str,
    title: str,
    output_file: str,
):
    """values is a list of quadruples: (label, W(n), Q(n), T(n)), where the multiplier 'n' is implicit"""

    # Goal: See slide 4 of lecture 'roofline'
    mpl.rcParams["axes.prop_cycle"] = mpl.cycler(color=COLOR_LIST)
    mpl.rcParams["figure.figsize"] = [8, 5]

    logging.info("generating roofline plot...")

    # set up bound visualizations
    _, ax = plt.subplots()

    # plot performance bounds
    pi_color = next(ax._get_lines.prop_cycler)["color"]
    piv_color = next(ax._get_lines.prop_cycler)["color"]
    b_color = next(ax._get_lines.prop_cycler)["color"]
    plt.axhline(
        y=pi, linewidth=1, color=pi_color
    )
    plt.axhline(
        y=pi_simd,
        linewidth=1,
        color=piv_color,
    )
    plt.axline(
        xy1=(pi / beta, pi),
        xy2=(pi_simd / beta, pi_simd),
        linewidth=1,
        color=b_color,
    )
    ax.annotate('P ≤ π', xy=(1.01,pi), xytext=(0.0, -5), xycoords=('axes fraction', 'data'), textcoords='offset points', color=pi_color)
    ax.annotate('P ≤ βI', xy=(pi / beta + (pi_simd / beta - pi / beta) / 2,pi + (pi_simd - pi) / 2), xytext=(-48.0, 0.0), xycoords='data', textcoords='offset points', color=b_color)
    ax.annotate('P ≤ π-SIMD', xy=(1.01,pi_simd), xytext=(0.0, -5), xycoords=('axes fraction', 'data'), textcoords='offset points', color=piv_color)

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
            cache_line_size = 64
            l3_total_size = 8388608
            if USE_Q_ESTIMATE:
                data_size = 8
                bytes_list = [
                    estimate_q(n, data_size, l3_total_size, cache_line_size)
                    for n in n_list
                ]
                _ = reader.__next__()  # l3 cache misses (on cold cache)
            else:
                l3_misses_list = reader.__next__()
                logging.debug(f"l3-misses: {l3_misses_list}")
                bytes_list = [
                    m * 64 for m in l3_misses_list
                ]  # multiply num misses by bytes transfered per miss
                logging.debug(f"--> bytes transfered: {bytes_list}")
            _ = reader.__next__()  # l2 cache misses (on cold cache)
            _ = reader.__next__()  # l1 cache misses (on cold cache)

        # compute performance (flops = 2 * n^3)
        i_list = list()
        p_list = list()
        for (n, c, _, Q) in zip(n_list, cycles_list, runs_list, bytes_list):
            logging.debug(f"---- For n = {n} ----")
            if Q == 0:
                continue
            W = 2 * n * n * n
            logging.debug(f"\tW = {W}")
            logging.debug(f"\tQ = {Q}")
            I = W / Q
            # P = (W / c) / 8 if bit_packed else W / c
            P = W / c
            logging.debug(f"\t=> I = {round(I, 2)}")
            logging.debug(f"\t=> P = {round(P, 2)}")
            i_list.append(I)
            p_list.append(P)

        # ensure scatter points and lines have the same color
        color = next(ax._get_lines.prop_cycler)["color"]
        # scatter points such that the same line has different pointsizes
        plt.scatter(
            i_list,
            p_list,
            marker="o",
            s=[5 + 8 * i for i in range(len(i_list))],
            label=label,
            color=color,
        )
        # plot the point such that we have lines connecting the points
        plt.plot(
            i_list,
            p_list,
            marker="o",
            markersize=1,
            label=label,
            color=color,
        )

    # configure plot
    plt.ylabel("P(n) [flops/cycle]")
    plt.xlabel("I(n) [bytes/cycle]")
    plt.xscale("log", base=2)
    plt.yscale("log", base=2)
    plt.grid(True, which="both", axis="both")
    plt.title(title)

    # reorder legend and dont add scatter to the legend
    handles, labels = plt.gca().get_legend_handles_labels()
    handles, labels = zip(
        *sorted(
            list(
                filter(
                    lambda t: not isinstance(t[0], mpl.collections.PathCollection),
                    zip(handles, labels),
                )
            ),
            key=lambda t: t[0]._y[0] if t[0]._y is not None else 100,
            reverse=True,
        )
    )
    plt.legend(handles, labels, loc="center left", bbox_to_anchor=(1, 0.7))
    plt.tight_layout()

    logging.info("storing results to {}...".format(output_file))
    outfile = "{}/{}_roof.eps".format(plots_dir, output_file)
    plt.savefig(outfile)
    plt.clf()


if __name__ == "__main__":
    args = parser.parse_args()
    roofline_plot(
        args.data,
        args.peak,
        args.simd_peak,
        args.beta,
        args.bit_packed,
        args.labels,
        args.plot,
        args.title,
        args.output,
    )
