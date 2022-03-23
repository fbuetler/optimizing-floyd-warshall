import argparse
from cProfile import label
import csv
import os

import matplotlib.pyplot as plt

parser = argparse.ArgumentParser()
parser.add_argument("-d", "--data", help="csv file with data", type=str, required=True)
parser.add_argument(
    "-p", "--plot", help="directory to save the plots", type=str, required=True
)
parser.add_argument("-t", "--title", help="title for the plot", type=str, required=True)

args = parser.parse_args()

data_file = args.data
plots_dir = args.plot
title = args.title

performance_data = list()
with open(data_file) as f:
    reader = csv.reader(f, delimiter=",", quoting=csv.QUOTE_NONNUMERIC)
    n_list = reader.__next__()
    runs_list = reader.__next__()
    cycles_list = reader.__next__()

plt.plot(n_list, runs_list, "-o", label="runs")
plt.plot(n_list, cycles_list, "-o", label="cycles")

plt.xticks(n_list)

runs_max = max(runs_list)
cycles_max = max(cycles_list)
ymax = max(runs_max, cycles_max)
plt.ylim([0, max(2, ymax + 0.1 * ymax)])

plt.grid(True, which="major", axis="y")

plt.legend()

plt.title(title)
plt.xlabel("n")
plt.ylabel("cycles")

data_file_name = os.path.basename(data_file)
plt.savefig("{}/{}.png".format(plots_dir, data_file_name))
