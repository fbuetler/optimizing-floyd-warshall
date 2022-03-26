import argparse
import math
import os
import subprocess

parser = argparse.ArgumentParser()
parser.add_argument("-o", "--output-folder", help="path to output folder for all graph files")
parser.add_argument("--min-n", help="minimum n for the generated testcases", type=int)
parser.add_argument("--max-n", help="maximum n for the generated testcases", type=int)
parser.add_argument("-d", "--density", action="count", help="increase density (not passed m=n log(n), -d: m=n log(n)^2, -dd: m=n ln(n)^2, -ddd: m=n^2/2")
parser.add_argument("--exp2", help="double n in every step (cannot be used with --step)", action="store_true")
parser.add_argument("--step", help="step by which to increase n in every step (cannot be used with --exp2)", type=int)

args = parser.parse_args()

subprocess.call(["mkdir", "-p", args.output_folder])

n = args.min_n
min_weight = 1
max_weight = math.floor(math.log(n))

while n <= args.max_n:
    if args.density == 3:
        m = math.floor((n*n) / 2)
    elif args.density == 2:
        m = n * math.floor(math.pow(math.log(n*n),2))
    elif args.density == 1:
        m = n * math.floor(math.pow(math.log(n),2))
    else:
        m = n * math.floor(math.log(n))

    folder = "{}/n{}".format(args.output_folder, n)
    subprocess.call(["mkdir", "-p", folder])
    subprocess.call(["python3", "generator/graph_generator.py", "-d", "-n", "{}".format(n), "-m", "{}".format(m), "-o", "{}/graph_n{}_e{}_min{}_max{}_dense{}_connected.in.txt".format(folder, n, m, min_weight, max_weight, args.density), "--min-weight", "{}".format(min_weight), "--max-weight", "{}".format(max_weight), "--connected"])

    if args.exp2:
        n *=2
    else:
        n += args.step
