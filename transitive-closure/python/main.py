#!/usr/bin/python
'''reference implementation of transitive closure in python'''
import sys
import getopt


def read_boolean_anw(path: str) -> list[list[bool]]:
    '''read a function graph from the provided path (in anw-style)'''
    with open(path, 'r') as f:
        lines = f.readlines()
        n = int(lines[0])
        e = int(lines[1])
        assert len(lines) == e + 2, \
            f'{e} edges announced, but {len(lines) - 2} provided'
        res = [[False for i in range(n)] for j in range(n)]
        for l in lines[2:]:
            x, y = l.split(',')
            res[int(x)][int(y)] = True
        return res


def read_boolean(path: str) -> list[list[bool]]:
    '''read a function graph from the provided path'''
    with open(path, 'r') as f:
        lines = f.readlines()
        n = int(lines[0])
        def mapf(x):
            try:
                tmp = float(x)
                return True
            except: return False
        return [list(map(mapf,
                         l.split(',')[:n])) for l in lines[1:]]


def write_boolean(path: str, c: list[list[bool]]) -> None:
    '''write the weighted matrix to the target path'''
    with open(path, 'w') as f:
        for line in c:
            for reach in line:
                if reach:
                    f.write('1')
                f.write(',')
            f.write('\n')


def transitive_closure(C: list[list[bool]], n: int) -> list[list[bool]]:
    '''compute the transitive closure for a given n-n matrix C'''
    for k in range(n):
        for i in range(n):
            for j in range(n):
                C[i][j] = C[i][j] or (C[i][k] and C[k][j])
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
        c = read_boolean(in_file)
    except Exception as e:
        print(f'unable to read file {in_file}: {e}')
        exit()
    n = len(c)
    transitive_closure(c, n)
    try:
        write_boolean(out_file, c)
    except Exception as e:
        print(f'unable to write to file {out_file}: {e}')


if __name__ == '__main__':
    main(sys.argv[1:])
