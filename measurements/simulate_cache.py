import argparse
import logging
from typing import List

logging.basicConfig(level=logging.INFO)

cache = {}

parser = argparse.ArgumentParser(description='analyzes the caching behavior of a program')
parser.add_argument(
    "-size", "--cache-size", help="cache size in bytes", type=int, required=True
)
parser.add_argument(
    "-e", "--cache-assoc", help="cache associativity", type=int, required=True
)
parser.add_argument(
    "-b", "--cache-line", help="cache line size in bytes", type=int, required=True
)
parser.add_argument(
    "-d",
    "--data-size",
    help="size in bytes of a single matrix element",
    type=int,
    required=True
)
parser.add_argument("-n", "--input-size", help="input size", type=int, nargs='+', required=True)


def init_cache(S: int, E: int, B: int):
    """ - S is the number of cache sets
        - E is the cache associativity
        - B is the cache line size in bytes
    """
    cache["S"] = S
    cache["E"] = E
    cache["B"] = B
    cache['hits'] = {}
    cache['hits']['total'] = 0
    cache['misses'] = {}
    cache['misses']['total'] = 0
    for s in range(S):
        cache[s] = {}
        for e in range(E):
            cache[s][e] = {}
            cache[s][e]['tag'] = -1
            cache[s][e]['t'] = -1

def reset_cache():
    for k in cache['hits'].keys():
        cache['hits'][k] = 0
        cache['misses'][k] = 0
    for s in range(cache['S']):
        cache[s] = {}
        for e in range(cache['E']):
            cache[s][e] = {}
            cache[s][e]['tag'] = -1
            cache[s][e]['t'] = -1

def access(i: int, j: int, n:int, data_size: int, t: int, operation: str, op_id: int) -> int:
    # compute tag and set identifiers
    # HACK: Assume cache line size to be divisible by sizeof(datatype)
    tag = (i * n + j) // (cache["B"]//data_size)
    set = tag % cache["S"]

    if op_id not in cache['hits']:
        cache['hits'][op_id] = 0
        cache['misses'][op_id] = 0

    # simulate cache access
    hit = False
    for e in range(cache["E"]):
        if cache[set][e]['tag'] == tag:
            hit = True
            cache['hits'][op_id] += 1
            cache['hits']['total'] += 1
            cache[set][e]['t'] = t
            break
    if not hit:
        cache['misses'][op_id] += 1
        cache['misses']['total'] += 1

        # find LRU and replace
        min_e = 0
        for e in range(1, cache["E"]):
            if cache[set][e]['t'] < cache[set][min_e]['t']:
                min_e = e
        cache[set][min_e]['tag'] = tag
        cache[set][min_e]['t'] = t

    # log event
    hm = 'H' if hit else 'M'
    logging.debug(f"{operation} to ({i}, {j}): {hm} in line {set}")

    # return
    return t+1


def output_cache(n:int, data_size: int):
    # output number of hits and misses
    logging.info(f"Total: n={n} | #hits: {cache['hits']['total']} | #misses: {cache['misses']['total']}")
    for op_id in cache['hits'].keys():
        if op_id != 'total':
            logging.info(f"Op {op_id}:  n={n} | #hits: {cache['hits'][op_id]} | #misses: {cache['misses'][op_id]}")

    # output cache contents if logging level set to DEBUG
    if logging.getLogger().isEnabledFor(logging.DEBUG):
        elems_per_line = cache["B"]//data_size

        for s in range(cache['S']):
            for e in range(cache['E']):
                tag = cache[s][e]['tag']
                if tag == -1:
                    state = ' '.join('-' for _ in range(elems_per_line))
                    logging.debug(f'line{s}: {state}')
                else:
                    def indices(ind):
                        j = ind % n
                        i = (ind - j) // n
                        return (i, j) if i < n and j < n else ('-', '-')
                    state = ' '.join(f'C{i}{j}' for (i,j) in [indices(tag * elems_per_line + offset) for offset in range(elems_per_line)])
                    logging.debug(f'line {s}, set {e}: {state}')

def naive(n: int, data_size: int):
    # HACK: Caching behavior for naive, unrolled and vector implementations should be the same
    t = 0
    for k in range(n):
        for i in range(n):
            for j in range(n):
                logging.debug('------------------------')
                t = access(i, k, n, data_size, t, 'R', 1) # (1)
                t = access(k, j, n, data_size, t, 'R', 2) # (2)
                t = access(i, j, n, data_size, t, 'R', 3) # (3)
    output_cache(n, data_size)

if __name__ == "__main__":
    args = parser.parse_args()
    e = args.cache_assoc
    b = args.cache_line
    s = args.cache_size // (e * b)
    logging.info(f'initializing cache with params (S,E,B) = ({s},{e},{b})...')
    init_cache(s,e,b)
    for n in args.input_size:
        naive(n, args.data_size)
        reset_cache()