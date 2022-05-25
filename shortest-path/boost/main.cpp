#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/property_maps/constant_property_map.hpp>
#include <boost/graph/floyd_warshall_shortest.hpp>
#include <boost/graph/directed_graph.hpp>
#include <boost/graph/exterior_property.hpp>

#ifdef __x86_64__
#include "tsc_x86.h"
#endif

#define NUM_RUNS 16
#define CYCLES_REQUIRED 1e8
#define FREQUENCY 2.3e9
#define CALIBRATE

// types
// graph type
typedef boost::property<boost::edge_weight_t, double> edge_weight_property;
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS,
                              boost::no_property, edge_weight_property>
    Graph;
typedef boost::property_map<Graph, boost::edge_weight_t>::type weight_map;

// matrix type
typedef boost::exterior_vertex_property<Graph, double> distance_property;
typedef distance_property::matrix_type distance_matrix;
typedef distance_property::matrix_map_type distance_matrix_map;

// write a distance-matrix to target filename
void output_matrix(char *filename, distance_matrix &matr, int N)
{
  fprintf(stderr, "outputting shortest-path matrix to %s...\n", filename);
  FILE *output_f = fopen(filename, "w+");
  fprintf(output_f, "%d\n", N);
  for (int i = 0; i < N; i++)
  {
    for (int j = 0; j < N; j++)
    {
      if (matr[i][j] == std::numeric_limits<double>::max())
      {
        // do nothing - infinity
      }
      else
      {
        fprintf(output_f, "%.2f", matr[i][j]);
      }
      if (j < N - 1)
      {
        fputc(',', output_f);
      }
      else
      {
        fputc('\n', output_f);
      }
    }
  }
}

#ifdef __x86_64__
/*
 * Timing function based on the TimeStep Counter of the CPU.
 *
 * We don't particularly care about the input, so we just
 * run FW on the same matrix C for each run even though it
 * modifies it in-place.
 *
 * The function returns the average number of cycles per run.
 */
unsigned long long rdtsc(Graph G, distance_matrix_map map, weight_map weights)
{
  int i, num_runs;
  myInt64 cycles;
  myInt64 start;
  num_runs = NUM_RUNS;

  /*
   * The CPUID instruction serializes the pipeline.
   * Using it, we can create execution barriers around the code we want to time.
   * The calibrate section is used to make the computation large enough so as to
   * avoid measurements bias due to the timing overhead.
   */
#ifdef CALIBRATE
  while (num_runs < (1 << 14))
  {
    start = start_tsc();
    for (i = 0; i < num_runs; ++i)
    {
      floyd_warshall_all_pairs_shortest_paths(G, map, boost::weight_map(weights));
    }
    cycles = stop_tsc(start);

    if (cycles >= CYCLES_REQUIRED)
      break;

    num_runs *= 2;
  }
#endif

  fprintf(stderr, "#runs: ");
  printf("%d\n", num_runs);

  /*
   * Alternatives to the current approach:
   * - Measure #cycles for each run separately
   *  --> Suffers from timing bias and requires cache warmup
   *  --> Allows for more fine-grained statistics, e.g. median, variance, etc.
   */

  start = start_tsc();
  for (i = 0; i < num_runs; ++i)
  {
    floyd_warshall_all_pairs_shortest_paths(G, map, boost::weight_map(weights));
  }
  cycles = stop_tsc(start) / num_runs;

  return cycles;
}
#endif

int main(int argc, char **argv)
{
  if (argc != 3)
  {
    fprintf(stderr, "incorrect number of arguments\n");
    fprintf(stderr, "call as: ./main input_filename output_filename\n");
    return -1;
  }

  // read matrix
  FILE *input_f = fopen(argv[1], "r");

  int N; // num nodes
  if (fscanf(input_f, "%d ", &N) < 1)
  {
    fprintf(stderr, "malformed input: couldn't match number N of vertices\n");
    return -1;
  }

  // create graph
  Graph G(N);
  weight_map weights = boost::get(boost::edge_weight, G);
  for (int i = 0; i < N; i++)
  {
    char inputValue[100];
    for (int j = 0; j < N; j++)
    {
      int numValues = fscanf(input_f, "%[^,\n]s", inputValue);
      if (numValues == 1)
      {
        auto e = boost::add_edge(i, j, G).first;
        weights[e] = strtof(inputValue, NULL);
      }
      fgetc(input_f); // skip ',' or '\n'
    }
  }
  distance_matrix dist(N);
  distance_matrix_map map(dist, G);

  // algorithm call
  bool valid = floyd_warshall_all_pairs_shortest_paths(G, map,
                                                       boost::weight_map(weights));

  if (!valid)
  {
    fprintf(stderr, "invalid graph provided: negative cycle encountered!\n");
    return 1;
  }

  output_matrix(argv[2], dist, N);

#ifdef __x86_64__
  fprintf(stderr, "finding shortest paths...\n");
  unsigned long long r = rdtsc(G, map, weights);
  fprintf(stderr, "#cycles on avg: ");
  printf("%llu\n", r);
#endif

  return 0;
}
