import argparse
import csv
import os

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
    type=str,
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


def main(
    data_file_list,
    label_list,
    plots_dir,
    title,
    peak,
):

    if len(data_file_list) != len(label_list):
        raise Exception(
            "List of labels must have the same length as the list of data file names: data had len {}, labels {}".format(
                len(data_file_list), len(label_list)
            )
        )

    data_label_list = zip(data_file_list, label_list)

    if peak != 0.0:
        _, ax = plt.subplots()
        plt.axhline(
            y=peak,
            label="P ≤ π",
            linewidth=1,
            color=next(ax._get_lines.prop_cycler)["color"],
        )

    mpl.rcParams["axes.prop_cycle"] = mpl.cycler(
        color=COLOR_LIST
    )  # sets colors using given cycle

    perf_max = 0.0

    for data_file, label in data_label_list:
        performance_data = list()
        with open(data_file) as f:
            reader = csv.reader(f, delimiter=",", quoting=csv.QUOTE_NONNUMERIC)
            n_list = reader.__next__()
            runs_list = reader.__next__()
            cycles_list = reader.__next__()

        # compute performance (flops = n^3)
        perf_list = list()
        for (n, c, r) in zip(n_list, cycles_list, runs_list):
            perf_list.append((2 * n * n * n) / c)

        plt.plot(n_list, perf_list, label=label, marker="^")

        perf_max = max(perf_list + [perf_max])

    plt.ylim(0, max(perf_max + 0.1 * perf_max, peak + 0.1 * peak))

    plt.grid(True, which="major", axis="y")

    plt.legend()

    plt.title(title)
    plt.xlabel("n")
    plt.semilogx(base=2)
    plt.ylabel("[flops/cycle]")

    data_file_name = os.path.basename(data_file).replace(".csv", "")
    outfile = "{}/{}_perf.png".format(plots_dir, data_file_name)
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
    )
