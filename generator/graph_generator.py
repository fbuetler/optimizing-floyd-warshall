import argparse
import random
from networkx import gnm_random_graph, is_connected, negative_edge_cycle, adjacency_matrix

parser = argparse.ArgumentParser()
parser.add_argument("-n", "--nodes", help="number of nodes in the graph", type=int, default=30)
parser.add_argument("-m", "--edges", help="number of edges in the graph", type=int, default=42)
parser.add_argument("-d", "--directed", help="is the graph directed", action="store_true")
parser.add_argument("-o", "--output", help="output file name", default="graph.txt")
parser.add_argument("--min-weight", help="minimum edge weight", type=float, default=0.0)
parser.add_argument("--max-weight", help="maximum edge weight", type=float, default=10.0)
parser.add_argument("--connected", help="Generated graph must be connected", action="store_true")
parser.add_argument("--no-neg-cycle", help="Generated graph must not contain a negative cycle", action="store_true")

args = parser.parse_args()

n = args.nodes
m = args.edges

G = None

redo_graph = True
while redo_graph:
    redo_graph = False

    G = gnm_random_graph(n, m, None, args.directed)

    if args.connected and not is_connected(G):
        redo_graph = True
        continue

    for u, v in G.edges:
        G[u][v]["weight"] = random.normalvariate((args.max_weight - args.min_weight) / 2.0, (args.max_weight - args.min_weight) / 8)

    if args.no_neg_cycle and negative_edge_cycle(G):
        redo_graph = True
        continue

with open(args.output, "w") as f:
    f.write("{}\n".format(n))
    for u in range(n):
        for v in range(n):
            if v != 0:
                f.write(",")

            try:
                f.write("{:.6f}".format(G[u][v]["weight"]))
            except KeyError:
                # do nothing
                f.write("")
        
        f.write("\n")
