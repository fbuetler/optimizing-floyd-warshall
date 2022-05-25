#include <stdio.h>
#include <iostream>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/transitive_closure.hpp>

#ifdef __x86_64__
#include "tsc_x86.h"
#endif

#define NUM_RUNS 16
#define CYCLES_REQUIRED 1e8
#define FREQUENCY 2.3e9
#define CALIBRATE

using namespace std;

// types
typedef boost::adjacency_matrix<boost::directedS> Graph;

// utility function to print matrices
void printMatrix(Graph &G, long int N)
{
  for (long int i = 0; i < N; i++)
  {
    for (long int j = 0; j < N; j++)
    {
      if (boost::edge(i, j, G).second)
      {
        fprintf(stderr, "1, ");
      }
      else
      {
        fprintf(stderr, " , ");
      }
    }
    fprintf(stderr, "\n");
  }
}

// write matrix to file
void output_matrix(char *filename, Graph &G, long int N)
{
  fprintf(stderr, "outputting transitive closure matrix to %s...\n", filename);
  FILE *output_f = fopen(filename, "w+");
  fprintf(output_f, "%d\n", N);
  for (long int i = 0; i < N; i++)
  {
    for (long int j = 0; j < N; j++)
    {
      if (boost::edge(i, j, G).second)
      {
        fprintf(output_f, "1");
      }
      else
      {
        fprintf(output_f, "0");
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
  fclose(output_f);
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
unsigned long long rdtsc(Graph G)
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
      boost::warshall_transitive_closure(G);
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
    boost::warshall_transitive_closure(G);
  }
  cycles = stop_tsc(start) / num_runs;

  return cycles;
}
#endif

// Main function: read matrix, run algorithm, write result
int main(int argc, char **argv)
{
  if (argc != 3)
  {
    fprintf(stderr, "incorrect number of arguments\n");
    fprintf(stderr, "call as: ./main input_filename output_filename\n");
    return -1;
  }
  cout << argv[1] << endl;
  FILE *input_f = fopen(argv[1], "r");

  // read graph
  int n; // num nodes
  if (fscanf(input_f, "%d ", &n) < 1)
  {
    fprintf(stderr, "malformed input: couldn't match number N of vertices\n");
    return -1;
  }

  long int N = n;

  Graph G(N); // input graph
  for (long unsigned int i = 0; i < N; i++)
  {
    char inputValue[100];
    for (long unsigned int j = 0; j < N; j++)
    {
      int numValues = fscanf(input_f, "%[^,\n]s", inputValue);
      if (numValues == 1)
      {
        boost::add_edge(i, j, G);
      }
      fgetc(input_f); // skip ',' or '\n'
    }
  }

  // printMatrix(G, N);
  boost::warshall_transitive_closure(G);
  // printMatrix(G, N);

  output_matrix(argv[2], G, N);

#ifdef __x86_64__
  fprintf(stderr, "finding transitive closure...\n");
  unsigned long long r = rdtsc(G);
  fprintf(stderr, "#cycles on avg: ");
  printf("%llu\n", r);
#endif

  return 0;
}
