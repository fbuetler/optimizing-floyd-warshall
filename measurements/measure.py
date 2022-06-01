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
    "-c", "--testcases", help="testcases to process", type=str, default=""
)
parser.add_argument(
    "-o",
    "--output",
    help="path of the genereated csv file",
    type=str,
    required=True,
)
parser.add_argument(
    "-tb",
    "--allow-turbo-boost",
    help="allow to run the measurements with turbo boost",
    action="store_true",
)


def main(
    binary,
    testsuite_dir,
    testcases_raw,
    out_filepath,
    turbo_boost_allowed,
):
    if not turbo_boost_allowed:
        with open("/sys/devices/system/cpu/intel_pstate/no_turbo", "r") as f:
            if f.readline().strip() == "0":
                raise Exception("disable the turbo boost")

    if len(testcases_raw) == 0:
        testcases = []
    else:
        testcases = testcases_raw.split(",")

    nodes_list = list()
    runs_list = list()
    cycles_list = list()
    l3_list = list()
    l2_list = list()
    l1_list = list()
    tcs = os.listdir(testsuite_dir)
    for tc in tcs:
        if len(testcases) != 0 and tc not in testcases:
            continue
        print("Processing testcase: {}".format(tc))

        # get input file
        testcase_dir = os.path.join(testsuite_dir, tc)
        input_files = [
            f for f in os.listdir(testcase_dir) if re.match(r".*\.in\.txt$", f)
        ]
        if len(input_files) != 1:
            print("=== OUTPUT ===")
            for files in input_files:
                print(files)
            print("==============")
            raise Exception(
                "Unexpected number of testcase input files found. Expexted exactly one."
            )
        input_file = input_files[0]

        # generate output file
        file_parts = input_file.split(".")
        if len(file_parts) != 3:
            print("=== OUTPUT ===")
            for parts in file_parts:
                print(parts)
            print("==============")
            raise Exception(
                "Unexpected number of file paths found. Expected exactly 3."
            )
        output_file = "{}.out.{}".format(file_parts[0], file_parts[2])

        # run binary
        args = (
            binary,
            os.path.join(testcase_dir, input_file),
            os.path.join(testcase_dir, output_file),
        )
        #print(args)
        popen = subprocess.Popen(
            args, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL
        )
        popen.wait()
        lines = popen.stdout.read().decode("utf-8").split("\n")
        if len(lines) != 5:
            # numbe of runs/number of cycles/newline
            print("=== OUTPUT ===")
            for line in lines:
                print(line)
            print("==============")
            raise Exception("Unexpected number of lines found. Expected exactly 5.")

        # parse measurements
        print("Processed testcase: {}".format(tc))
        nodes = int(re.search(r"^graph_n(\d*)_e\d*.*", input_file).group(1))
        runs = int(lines[0].strip())
        cycles = int(lines[1].strip())
        l3_misses = int(lines[2].strip())
        l2_misses = int(lines[3].strip())
        l1_misses = int(lines[4].strip())

        print("\tNumber of nodes: {}".format(nodes))
        print("\tNumber of runs: {}".format(runs))
        print("\tNumber of cycles: {}".format(cycles))
        print("\tNumber of l3 cache misses: {}".format(l3_misses))
        print("\tNumber of l2 cache misses: {}".format(l2_misses))
        print("\tNumber of l1-d cache misses: {}".format(l1_misses))

        nodes_list.append(nodes)
        runs_list.append(runs)
        cycles_list.append(cycles)
        l3_list.append(l3_misses)
        l2_list.append(l2_misses)
        l1_list.append(l1_misses)

    # sort by nodes ascending
    nodes_list_sorted, runs_list_sorted, cycles_list_sorted, l3_list_sorted, l2_list_sorted, l1_list_sorted = zip(
        *sorted(zip(nodes_list, runs_list, cycles_list, l3_list, l2_list, l1_list))
    )

    binary_name = os.path.basename(binary)
    testsuite_name = os.path.basename(testsuite_dir)
    with open("{}{}.csv".format(out_filepath, str(testcases_raw)), "w") as f:
        writer = csv.writer(f)
        writer.writerows([nodes_list_sorted, runs_list_sorted, cycles_list_sorted, l3_list_sorted, l2_list_sorted, l1_list_sorted])


if __name__ == "__main__":
    args = parser.parse_args()
    main(
        args.binary,
        args.testsuite,
        args.testcases,
        args.output,
        args.allow_turbo_boost,
    )
