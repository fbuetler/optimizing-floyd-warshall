#!/usr/bin/python
"""Use to compare two matrices."""
import sys
import getopt
from os import walk
from pathlib import Path
from math import inf
import numpy as np
import graphviz as gv


def read_matrix(path: str) -> np.ndarray:
    """Read a real-valued matrix from a file."""
    with open(path, "r") as f:
        lines = f.readlines()
        n = int(lines[0])
        assert (
            len(lines) == n + 1
        ), f"{n} lines announced, but {len(lines) - 1} provided"

        def maf(x):
            try:
                return float(x)
            except Exception:
                return inf

        res = [list(map(maf, ln.split(",")))[:n] for ln in lines[1:]]
        return np.array(res)


def compare(
    m1: np.ndarray, m2: np.ndarray, tgt: str = "", precision: float = 0.000_000_1
) -> bool:
    """Compare two matrices.

    If :m1: and :m2: differ from eachother, return False, otherwise True.
    If a target path != '' is specified, a visualization is saved to :tgt:
    """
    comp = np.isclose(m1, m2, atol=precision)
    if tgt != "":
        G = gv.Digraph()
        for i, j in np.ndindex(m1.shape):
            if comp[i][j]:  # equal values
                if (w_cur := m1[i][j]) != inf:  # edge exists
                    G.edge(f"{i}", f"{j}", label=f"{w_cur:.4g}")
            else:
                lbl = f"{abs(m1[i][j] - m2[i][j]):.4g}"
                G.edge(f"{i}", f"{j}", label=lbl, color="red", fontcolor="red")
        G.render(tgt, format="png")
    return np.all(comp)


def compare_recursive(path: str, precision: float = 0.000_000_1) -> bool:
    """Recursively compare the files within the provided directory.

    :path: root directory to do comparisions in
    :precision: precision to use for comparisions (will be used for everything
                except .tc files)
    """
    print()
    suc, tot = 0, 0  # passes / total tests
    for (curpath, _, files) in walk(path):
        if not files:  # empty directory
            continue

        # find valid pairs
        files = sorted(files)
        print(
            f'Directory:{"testcases"+curpath.split("testcases")[1]:>40}:\n'
            f'==========={40*"="}='
        )
        for x, y in zip(files, files[1:]):
            if (stem := x.split(".")[0]) != y.split(".")[0]:
                continue  # different names
            px, py = Path(x), Path(y)
            if (
                len(px.suffixes) != 3
                or len(py.suffixes) != 3
                or px.suffixes[0] != py.suffixes[0]
            ):
                continue
            if (".out" not in px.suffixes) or (".ref" not in py.suffixes):
                continue  # not valid out-ref pair
            curprec = inf if px.suffixes[0] == ".tc" else precision
            res = compare(
                read_matrix(f"{curpath}/{x}"),
                read_matrix(f"{curpath}/{y}"),
                tgt="",
                precision=curprec,
            )
            msg = "\033[92mPASS\033[0m" if res else "\033[93mFAIL\033[0m"
            print(f'- {stem+px.suffixes[0]+":":45} {msg}')
            tot += 1
            suc += int(res)
        print()

    if tot == 0:
        print("No Testcases Found!")
        return 2
    print(f"{suc} of {tot} tests ({suc/tot:.0%}) passed!")

    return suc == tot


def main(argv: list) -> None:
    """Read two matrices from file, then compare them.

    Extra arguments (have to come before the two files to compare):
    -p or --precision= to set the precision to which the matrices are compared.
          Default is 0.000_000_1.
    -i or --img= to specify an image path. If provided, a visualization is
          saved to that location (with differing edges in red).
    -s or --silent to disable output message.
    -h or --help to print this message.
    -r or --recursive to iterate through a directory and compare files in it.
          In this case, only supply one argument - the root dir to do the
          comparisons in.

    :return: 0 if the two matrices are equal
             1 if they differ
             2 if an error occured (like unable to read matrices, or bad input)
             In case the recursive flag was used, the outputs are similar, but
             for every found pair of files (i.e. 0 -> all found files match).
    """
    # parse options
    try:
        opts, args = getopt.getopt(
            argv, "si:p:r", ["img=", "precision=", "silent", "recursive"]
        )
    except getopt.GetoptError:
        print("bad input")
        return 2
    # if len(args) != 2:
    #     print(f'invalid number of input files: {len(args)} given, '
    #           f'but 2 required')
    #     return(2)
    img = ""
    prec = 0.01
    silent = False
    rec = False
    for opt, arg in opts:
        if opt in ("--img", "-i"):
            img = arg
        elif opt in ("--precision", "-p") and arg == "inf":
            prec = inf
        elif opt in ("--precision", "-p"):
            prec = float(arg)
        elif opt in ("-s", "--silent"):
            silent = True
        elif opt in ("-r", "--recursive"):
            rec = True

    # recursive case?
    if rec:
        return int(not compare_recursive(args[0], prec))

    # otherwise, read input
    try:
        m1 = read_matrix(args[0])
        m2 = read_matrix(args[1])
    except Exception as e:
        print(f"unable to read input: {e}")
        return 2
    if m1.shape != m2.shape:
        print(f"incompatible matrix dimensions: {m1.shape} and {m2.shape}")
        return 2

    # compare
    res = compare(m1, m2, img, precision=prec)
    msg = f'{args[0]} and {args[1]} are {"" if res else "not "}equal!'
    if not silent:
        print(msg)

    return int(not res)


if __name__ == "__main__":
    if "--help" in sys.argv or "-h" in sys.argv:
        print(main.__doc__)
    else:
        exit(main(sys.argv[1:]))
