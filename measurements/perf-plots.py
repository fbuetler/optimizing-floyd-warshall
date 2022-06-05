import argparse
import csv
import pathlib

import matplotlib as mpl
import matplotlib.pyplot as plt

COLOR_LIST = [
    "#1b9e77",
    "#d95f02",
    "#7570b3",
    "#e7298a",
    "#66a61e",
    "#e6ab02",
    "#a6761d",
    "#666666",
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
    default=0.0,
)
parser.add_argument(
    "-vpi",
    "--simd-peak",
    help="maximum achievable performance in flops/cycle using SIMD instructions",
    type=float,
    default=0.0,
)
parser.add_argument("-o", "--output", help="output file name", type=str, required=True)


def main(
    data_file_list,
    label_choices,
    plots_dir,
    title,
    peak,
    peak_simd,
    output_file,
):

    if peak != 0.0:
        print(f"plotting non-SIMD performance limit at {peak} flops/cycle")
        _, ax = plt.subplots()
        plt.axhline(
            y=peak,
            label="P ≤ π",
            linewidth=1,
            color=next(ax._get_lines.prop_cycler)["color"],
        )

    if peak_simd != 0.0:
        print(f"plotting SIMD performance limit at {peak_simd} flops/cycle")
        _, ax = plt.subplots()
        plt.axhline(
            y=peak_simd,
            label="P ≤ π-SIMD",
            linewidth=1,
            color=next(ax._get_lines.prop_cycler)["color"],
        )

    mpl.rcParams["axes.prop_cycle"] = mpl.cycler(
        color=COLOR_LIST
    )  # sets colors using given cycle

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

        # compute performance (flops = 2 * n^3)
        perf_list = list()
        for (n, c, r) in zip(n_list, cycles_list, runs_list):
            perf_list.append((2 * n * n * n) / c)

        plt.plot(n_list, perf_list, label=label, marker="^")

        perf_max = max(perf_list + [perf_max])

    plt.ylim(
        0,
        max(
            [perf_max + 0.1 * perf_max, peak + 0.1 * peak, peak_simd + 0.1 * peak_simd]
        ),
    )

    plt.grid(True, which="major", axis="y")

    plt.legend()

    plt.title(title)
    plt.xlabel("n")
    plt.semilogx(base=2)
    plt.ylabel("[flops/cycle]")

    outfile = "{}/{}_perf.png".format(plots_dir, output_file)
    plt.savefig(outfile)
    plt.clf()

    print("stored performance plot to {}".format(outfile))


if __name__ == "__main__":
    args = parser.parse_args()
    main(
        args.data,
        args.labels,
        args.plot,
        args.title,
        args.peak,
        args.simd_peak,
        args.output,
    )
