#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>

#include "impl/tc.h"

#ifdef __x86_64__
#include "tsc_x86.h"
#endif

#define NUM_RUNS 1
#define CYCLES_REQUIRED 1e8
#define FREQUENCY 2.3e9
#define CALIBRATE

void printMatrix(char *C, int N)
{
  int bpl = ceil(N / 8.0); // bytes per matrix line
  for (int i = 0; i < N; i++)
  {
    for (int j = 0; j < N; j++)
    {
      int bind = j % 8;
      char cur = C[i * bpl + j / 8];
      if (cur & (1 << bind))
        fprintf(stderr, "1, ");
      else
        fprintf(stderr, "0, ");
    }
    fprintf(stderr, "\n");
  }
}

// TODO: Include cache warmup

/*
 * Runs the FW implementation once for testing purposes
 * Note that the matrix C is modified in-place
 */
void ref_output(char *C, int N)
{
  int err = floydWarshall(C, N);
  if (err != 0)
  {
    printf("implementation reported an error\n");
    exit(1);
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
double rdtsc(char *C, int N)
{
  int i, num_runs;
  myInt64 cycles;
  myInt64 start;
  num_runs = NUM_RUNS;

#ifdef CALIBRATE
  while (num_runs < (1 << 14))
  {
    start = start_tsc();
    for (i = 0; i < num_runs; ++i)
    {
      floydWarshall(C, N);
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
    floydWarshall(C, N);
  }
  cycles = stop_tsc(start) / num_runs;

  return (double)cycles;
}
#endif

void output_matrix(char *filename, char *C, int N)
{
  int bpl = ceil(N / 8.0); // bytes per matrix line
  fprintf(stderr, "outputting transitive closure matrix to %s...\n", filename);
  FILE *output_f = fopen(filename, "w+");
  fprintf(output_f, "%d\n", N);
  for (int i = 0; i < N; i++)
  {
    for (int j = 0; j < N; j++)
    {
      int bind = j % 8;
      char cur = C[i * bpl + j / 8];
      if (cur & (1 << bind))
        fprintf(output_f, "1");
      else
        fprintf(output_f, "0");
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

int main(int argc, char **argv)
{
  if (argc != 3)
  {
    fprintf(stderr, "incorrect number of arguments\n");
    fprintf(stderr, "call as: ./main input_filename output_filename\n");
    return -1;
  }

  FILE *input_f = fopen(argv[1], "r");

  int N; // num nodes
  if (fscanf(input_f, "%d ", &N) < 1)
  {
    fprintf(stderr, "malformed input: couldn't match number N of vertices\n");
    return -1;
  }

  fprintf(stderr, "allocating memory...\n");
  int bpl = ceil(N / 8.0);
  char *C = (char *)malloc(N * bpl * sizeof(char));
  fprintf(stderr, "parsing input matrix...\n");
  for (int i = 0; i < N; i++)
  {
    char inputValue[100];
    char curbyte = 0;
    for (int j = 0; j < N; j++)
    {
      int bj = j % 8;
      if (bj == 0)
        curbyte = 0; // new byte - set all 0

      int numValues = fscanf(input_f, "%[^,\n]s", inputValue);
      if (numValues == 1)
      {
        curbyte |= 1 << bj;
      }
      fgetc(input_f); // skip ',' or '\n'
      C[i * bpl + j / 8] = curbyte;
    }
  }
  fclose(input_f);

  char *D = (char *)malloc(N * bpl * sizeof(char));
  memcpy(D, C, N * bpl * sizeof(char));
  fprintf(stderr, "generating test output...\n");
  ref_output(D, N);
  char ref_output[256];
  sprintf(ref_output, "%s", argv[2]);
  output_matrix(ref_output, D, N);
  free(D);

#ifdef __x86_64__
  fprintf(stderr, "running optimized version...\n");
  double r = rdtsc(C, N);
  fprintf(stderr, "#cycles on avg: ");
  printf("%lf\n", r);
#endif

  free(C);
}
