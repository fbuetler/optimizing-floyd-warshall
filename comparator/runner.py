#!/usr/bin/python
"""Run a command against all inputs in a directory."""
from os import walk
from pathlib import Path
import subprocess
import argparse


def run_recursive(path: str, binary: str, ending: str):
    """Recursively run the provided binary against inputs found in path.

    For every file of the form "name.in.txt", execute the command
    ":binary: name.in.txt name:ending:". So :ending: should be something
    like ".fw.out.txt", ".tc.ref.txt", or something.
    """
    for (curpath, _, files) in walk(path):
        if not files:  # empty directory
            continue

        # find input-file(s)
        for f in files:
            p = Path(f)
            if p.suffixes != [".in", ".txt"]:  # not input-format
                continue

            stem = f.split(".")[0]
            in_path = f"{curpath}/{f}"
            out_path = f"{curpath}/{stem}{ending}"
            cmd = f"{binary} {in_path} {out_path}"

            print(f'running command: "{cmd}"')
            b_args = (binary, in_path, out_path)
            with subprocess.Popen(
                b_args, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL
            ) as popen:
                popen.wait()


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-b", "--binary", help="binary to run", type=str, required=True)
    parser.add_argument(
        "-a",
        "--algorithm",
        help="algorithm-id to use in file-ending",
        type=str,
        required=False,
    )
    parser.add_argument(
        "-o",
        "--output",
        help="output-type ('ref' or 'out') for file-ending",
        type=str,
        required=False,
    )
    parser.add_argument(
        "-d",
        "--directory",
        help="directory to search for inputs / " "generate outputs in",
        type=str,
        required=True,
    )
    args = parser.parse_args()
    if args.algorithm is None and args.output is None:
        run_recursive(args.directory, args.binary, "")
    else:
        run_recursive(
            args.directory, args.binary, f".{args.algorithm}.{args.output}.txt"
        )
