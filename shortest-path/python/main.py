#!/usr/bin/python
'''reference implementation of Floyd-Warshall in python'''
import sys
import getopt
from math import inf  # to represent non-connected nodes


def read_weighted_anw(path: str) -> list[list[float]]:
    '''read a weighted graph from the provided path (in anw style), convert it
    to adjacency matrix'''
    with open(path, 'r') as f:
        lines = f.readlines()
        n = int(lines[0])
        e = int(lines[1])
        assert len(lines) == e + 2, \
            f'{e} edges announced, but {len(lines) - 2} provided'
        res = [[inf for i in range(n)] for j in range(n)]
        for l in lines[2:]:
            x, y, w = l.split(',')
            res[int(x)][int(y)] = float(w)
        return res


def read_weighted(path: str) -> list[list[float]]:
    '''Read a real-valued matrix from a file. Its first line contains the
    number of columns / rows, the following lines contain the
    comma-separated distances (blank for non-connected)
    '''
    with open(path, 'r') as f:
        lines = f.readlines()
        n = int(lines[0])
        assert len(lines) == n + 1, \
            f'{n} lines announced, but {len(lines) - 1} provided'
        def maf(x):
            try: return float(x)
            except: return inf
        res = [list(
            map(maf,
                l.split(',')))[:n] for l in lines[1:]]
        return res


def write_weighted(path: str, c: list[list[float]]) -> None:
    '''write a real-valued matrix to the target path'''
    with open(path, 'w') as f:
        f.write(f'{len(c)}\n')
        for line in c:
            for dist in line:
                if dist != inf:
                    f.write(f'{dist:.2f}')
                f.write(',')
            f.write('\n')


def shortest_paths(C: list[list[float]], n: int) -> list[list[float]]:
    '''compute the shortest path costs between all vertex-pairs in a graph C, with
    n nodes
    '''
    for k in range(n):
        for i in range(n):
            for j in range(n):
                C[i][j] = min(C[i][j], C[i][k] + C[k][j])
    return C


def main(argv):
    try:
        opts, args = getopt.getopt(argv, 'hi:o:', ['in=', 'out='])
    except getopt.GetoptError:
        print('bad input')
        exit()
    in_file = ''
    out_file = ''
    for opt, arg in opts:
        if opt in ('-i', '--in'):
            in_file = arg
        elif opt in ('-o', '--out'):
            out_file = arg

    # call the functions
    c = None
    try:
        c = read_weighted(in_file)
    except Exception as e:
        print(f'unable to read file {in_file}: {e}')
        exit()
    n = len(c)
    shortest_paths(c, n)
    try:
        write_weighted(out_file, c)
    except Exception as e:
        print(f'unable to write to file {out_file}: {e}')


if __name__ == '__main__':
    main(sys.argv[1:])
