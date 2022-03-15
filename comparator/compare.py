#!/usr/bin/python
"""Use to compare two matrices."""
import sys
import getopt
from math import inf
import numpy as np
import graphviz as gv


def read_matrix(path: str) -> np.ndarray:
    """Read a real-valued matrix from a file."""
    with open(path, 'r') as f:
        lines = f.readlines()
        n = int(lines[0])
        assert len(lines) == n + 1, \
            f'{n} lines announced, but {len(lines) - 1} provided'

        def maf(x):
            try:
                return float(x)
            except Exception:
                return inf
        res = [list(
            map(maf,
                ln.split(',')))[:n] for ln in lines[1:]]
        return np.array(res)


def compare(m1: np.ndarray, m2: np.ndarray, tgt: str = '',
            precision: float = 0.000_000_1) -> bool:
    """Compare two matrices.

    If :m1: and :m2: differ from eachother, return False, otherwise True.
    If a target path != '' is specified, a visualization is saved to :tgt:
    """
    comp = np.isclose(m1, m2, atol=precision)
    if tgt != '':
        G = gv.Digraph()
        for i, j in np.ndindex(m1.shape):
            if comp[i][j]:  # equal values
                if (w_cur := m1[i][j]) != inf:  # edge exists
                    G.edge(f'{i}', f'{j}', label=f'{w_cur:.4g}')
            else:
                lbl = f'{abs(m1[i][j] - m2[i][j]):.4g}'
                G.edge(f'{i}', f'{j}', label=lbl, color='red', fontcolor='red')
        G.render(tgt, format='png')
    return np.all(comp)


def main(argv: list[str]) -> None:
    """Read two matrices from file, then compare them.

    Extra arguments (have to come before the two files to compare):
    -p or --precision= to set the precision to which the matrices are compared.
          Default is 0.000_000_1.
    -i or --img= to specify an image path. If provided, a visualization is
          saved to that location (with differing edges in red).
    -s or --silent to disable output message.
    -h or --help to print this message.

    :return: 0 if the two matrices are equal
             1 if they differ
             2 if an error occured (like unable to read matrices, or bad input)
    """
    # parse options
    try:
        opts, args = getopt.getopt(argv, 'si:p:',
                                   ['img=', 'precision=', 'silent'])
    except getopt.GetoptError:
        print('bad input')
        return(2)
    if len(args) != 2:
        print(f'invalid number of input files: {len(args)} given, '
              f'but 2 required')
        return(2)
    img = ''
    prec = 0.01
    silent = False
    for opt, arg in opts:
        if opt in ('--img', '-i'):
            img = arg
        elif opt in ('--precision', '-p') and arg == 'inf':
            print('hey')
            prec = inf
        elif opt in ('--precision', '-p'):
            prec = float(arg)
        elif opt in ('-s', '--silent'):
            silent = True

    # read input
    try:
        m1 = read_matrix(args[0])
        m2 = read_matrix(args[1])
    except Exception as e:
        print(f'unable to read input: {e}')
        return(2)
    if m1.shape != m2.shape:
        print(f'incompatible matrix dimensions: {m1.shape} and {m2.shape}')
        return(2)

    # compare
    res = compare(m1, m2, img, precision=prec)
    msg = f'{args[0]} and {args[1]} are {"" if res else "not "}equal!'
    if not silent:
        print(msg)

    return int(not res)


if __name__ == '__main__':
    if '--help' in sys.argv or '-h' in sys.argv:
        print(main.__doc__)
    else:
        exit(main(sys.argv[1:]))
