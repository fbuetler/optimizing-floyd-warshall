import argparse
import csv
import pathlib
from typing import Dict, List, Tuple
import matplotlib.pyplot as plt
import matplotlib as mpl
from cycler import cycler

COLOR_LIST = ['#1b9e77', '#d95f02', '#7570b3', '#e7298a',
              '#66a61e', '#e6ab02', '#a6761d', '#666666']


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
parser.add_argument("-vpi","--simd-peak", help="maximum achievable performance in flops/cycle using SIMD instructions", type=float, default=16.0)
parser.add_argument("-o", "--output", help="output file name", type=str, required=True)


def roofline_plot(
    data_file_list: List[str],
    pi: float,
    pi_simd: float,
    beta: float,
    label_choices: List[str],
    plots_dir: str,
    title: str,
    output_file: str):
    """ values is a list of quadruples: (label, W(n), Q(n), T(n)), where the multiplier 'n' is implicit """

    # Goal: See slide 4 of lecture 'roofline'
    mpl.rcParams['axes.prop_cycle'] = mpl.cycler(color=COLOR_LIST)

    print("generating roofline plot...")

    # set up bound visualizations
    _, ax = plt.subplots()
    ymax = pi_simd * 1.1

    # plot performance bounds
    plt.axhline(y=pi, label='P ≤ π', linewidth=1, color=next(
        ax._get_lines.prop_cycler)['color'])
    plt.axhline(y=pi_simd, label='P ≤ π-SIMD', linewidth=1,
                color=next(ax._get_lines.prop_cycler)['color'])
    plt.axline((pi/beta, pi), slope=beta, label='P ≤ βI', linewidth=1, color=next(ax._get_lines.prop_cycler)['color'])

    perf_max = 0.0

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
            bytes_list = reader.__next__()

        # compute performance (flops = 2 * n^3)
        i_list = list()
        p_list = list()
        min_n = (0,0,None)
        max_n = (0,0,None)
        for (n, c, _, Q) in zip(n_list, cycles_list, runs_list, bytes_list):
            W = (2 * n * n * n)
            I = W/Q
            P = W / c
            i_list.append(I)
            p_list.append(P)
            if min_n[2] is None or n < min_n[2]:
                min_n = (I, P, n)
            if max_n[2] is None or n > max_n[2]:
                max_n = (I, P, n)

        plt.annotate(int(min_n[2]), xy=(min_n[0], min_n[1]), xytext=(min_n[0], min_n[1]+0.1))
        plt.annotate(int(max_n[2]), xy=(max_n[0], max_n[1]), xytext=(max_n[0], max_n[1]+0.2))

        plt.plot(i_list, p_list, marker='o', label=label)

        perf_max = max(p_list + [perf_max])

    # configure plot
    plt.xlabel('I(n) [flops/cycle]')
    plt.ylabel('P(n) [flops/cycle]')
    plt.xscale('log', base=2)
    plt.yscale('log', base=2)
    plt.grid(True, which="both", axis="both")
    plt.title(title)
    plt.legend(loc='center left', bbox_to_anchor=(
        1, 0.5))  # TODO: Label lines directly
    plt.tight_layout()

    print("storing results to {}...".format(output_file))
    outfile = "{}/{}_roof.png".format(plots_dir, output_file)
    plt.savefig(outfile)
    plt.clf()

if __name__ == "__main__":
    args = parser.parse_args()
    roofline_plot(
        args.data,
        args.peak,
        args.simd_peak,
        args.beta,
        args.labels,
        args.plot,
        args.title,
        args.output,
    )