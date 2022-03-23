import argparse
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
    for row in reader:
        performance_data.append(row)

plot_args = list()
for pd in performance_data:
    plot_args += [n_list, pd, "o-"]
plt.plot(*plot_args)

plt.xticks(n_list[0::2])

ymax = max([max(l) for l in performance_data])
plt.ylim([0, max(2, ymax + 0.1 * ymax)])

plt.grid(True, which="major", axis="y")

plt.legend("runs,cycles")

plt.title(title)
plt.xlabel("n")
plt.ylabel("cycles")

data_file_name = os.path.basename(data_file)
plt.savefig("{}/{}.png".format(plots_dir, data_file_name))
