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
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            fprintf(stderr, "%d, ", C[i * N + j]);
        }
        fprintf(stderr, "\n");
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
            warshall(C, N);
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
        warshall(C, N);
    }
    cycles = stop_tsc(start) / num_runs;

    return (double)cycles;
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

    FILE *input_f = fopen(argv[1], "r");

    int N; // num nodes
    if (fscanf(input_f, "%d ", &N) < 1)
    {
        fprintf(stderr, "malformed input: couldn't match number N of vertices\n");
        return -1;
    }

    fprintf(stderr, "allocating memory...\n");
    char *C = (char *)malloc(N * N * sizeof(char));
    fprintf(stderr, "parsing input matrix...\n");
    for (int i = 0; i < N; i++)
    {
        char inputValue[100];
        for (int j = 0; j < N; j++)
        {
            int numValues = fscanf(input_f, "%[^,\n]s", inputValue);
            if (numValues == 1)
            {
                C[i * N + j] = 1;
            }
            else
            {
                C[i * N + j] = 0;
            }
            fgetc(input_f); // skip ',' or '\n'
        }
    }
    fclose(input_f);

#ifdef __x86_64__
    fprintf(stderr, "finding transitive closure...\n");
    double r = rdtsc(C, N);
    fprintf(stderr, "#cycles on avg: ");
    printf("%lf\n", r);
#endif

    fprintf(stderr, "outputting transitive closure matrix to %s...\n", argv[2]);
    FILE *output_f = fopen(argv[2], "w+");
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            fprintf(output_f, "%d", C[i * N + j]);
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

    free(C);
}