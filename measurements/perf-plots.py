import argparse
import csv
import os
from cProfile import label

import matplotlib as mpl
import matplotlib.pyplot as plt

COLOR_LIST = ['#1b9e77', '#d95f02', '#7570b3', '#e7298a',
              '#66a61e', '#e6ab02', '#a6761d', '#666666']

parser = argparse.ArgumentParser(description="generate performance plot using data from a csv file")
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

mpl.rcParams['axes.prop_cycle'] = mpl.cycler(color=COLOR_LIST) # sets colors using given cycle

plt.plot(n_list, cycles_list, "-o", label="cycles", marker='^')

plt.xticks(n_list)

runs_max = max(runs_list)
cycles_max = max(cycles_list)
ymax = max(runs_max, cycles_max)
plt.ylim([0, max(2, ymax + 0.1 * ymax)])

plt.grid(True, which="major", axis="y")

plt.legend() # TODO: Label lines directly

plt.title(title)
plt.xlabel("n")
# plt.semilogx(base=2)
plt.ylabel("cycles")

data_file_name = os.path.basename(data_file).replace(".csv", "")
outfile = "{}/{}.png".format(plots_dir, data_file_name)
plt.savefig(outfile)
plt.clf()

print("stored performance plot to {}".format(outfile))