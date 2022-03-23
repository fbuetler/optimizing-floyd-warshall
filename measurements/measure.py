import argparse
import csv
import os
import re
import subprocess

parser = argparse.ArgumentParser()
parser.add_argument("-b", "--binary", help="binary to run", type=str, required=True)
parser.add_argument(
    "-t", "--testsuite", help="directory with testcases", type=str, required=True
)
parser.add_argument(
    "-m",
    "--measurements",
    help="directory to save the measurements",
    type=str,
    required=True,
)
parser.add_argument(
    "-tb",
    "--allow-turbo-boost",
    help="allow to run the measurements with turbo boost",
    action="store_true",
)

args = parser.parse_args()
binary = args.binary
testsuite_dir = args.testsuite
measurements_dir = args.measurements
turbo_boost_allowed = args.allow_turbo_boost

if not turbo_boost_allowed:
    with open("/sys/devices/system/cpu/intel_pstate/no_turbo", "r") as f:
        if f.readline().strip() == "0":
            raise Exception("disable the turbo boost")

nodes_list = list()
runs_list = list()
cycles_list = list()
for testcase in os.listdir(testsuite_dir):
    print("Processing testcase: {}".format(testcase))

    # get input file
    testcase_dir = os.path.join(testsuite_dir, testcase)
    input_files = [f for f in os.listdir(testcase_dir) if re.match(r".*\.in\.txt$", f)]
    if len(input_files) != 1:
        raise Exception(
            "Unexpected number of testcase input files found. Expexted exactly one."
        )
    input_file = input_files[0]

    # generate output file
    file_parts = input_file.split(".")
    if len(file_parts) != 3:
        raise Exception("Unexpected number of file paths found. Expected exactly 3.")
    output_file = "{}.out.{}".format(file_parts[0], file_parts[2])

    # run binary
    args = (
        binary,
        os.path.join(testcase_dir, input_file),
        os.path.join(testcase_dir, output_file),
    )
    popen = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
    popen.wait()
    lines = popen.stdout.read().decode("utf-8").split("\n")
    if len(lines) != 3:
        # numbe of runs/number of cycles/newline
        raise Exception("Unexpected number of lines found. Expected exactly 3.")

    # parse measurements
    print("Processed testcase: {}".format(testcase))
    nodes = int(re.search(r"^graph_n(\d*)_e\d*.*", input_file).group(1))
    runs = int(lines[0].strip())
    cycles = float(lines[1].strip())

    print("Number of nodes: {}".format(nodes))
    print("Number of runs: {}".format(runs))
    print("Number of cycles: {}".format(cycles))

    nodes_list.append(nodes)
    runs_list.append(runs)
    cycles_list.append(cycles)

binary_name = os.path.basename(binary)
testsuite_name = os.path.basename(testsuite_dir)
with open(
    "{}/{}_{}.csv".format(measurements_dir, binary_name, testsuite_name), "w"
) as f:
    writer = csv.writer(f)
    writer.writerows([nodes_list, runs_list, cycles_list])
