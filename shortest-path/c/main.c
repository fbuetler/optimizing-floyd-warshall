#include <malloc.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "impl/sp.h"

#ifdef __x86_64__
#include "tsc_x86.h"
#endif

#define NUM_RUNS 16
#define CYCLES_REQUIRED 1e8
#define FREQUENCY 2.3e9
#define CALIBRATE

void printMatrix(float *C, int N) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            fprintf(stderr, "%.2f, ", C[i * N + j]);
        }
        fprintf(stderr, "\n");
    }
}

/*
Copies data from one matrix to another.
Assumes matrices 'from' and 'to' point to separate memory locations.
*/
void copyMatrix(float *from, float *to, int N) {
#pragma ivdep
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            to[i * N + j] = from[i * N + j];
        }
    }
}

/*
 * Runs the FW implementation once for testing purposes
 * Note that the matrix C is modified in-place
 */
void ref_output(float *C, int N) {
    floydWarshall(C, N);
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
double rdtsc(float *C, int N) {
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
    while (num_runs < (1 << 14)) {
        start = start_tsc();
        for (i = 0; i < num_runs; ++i) {
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
    for (i = 0; i < num_runs; ++i) {
        floydWarshall(C, N);
    }
    cycles = stop_tsc(start) / num_runs;

    return (double)cycles;
}
#endif

void output_matrix(char *filename, float *C, int N) {
    fprintf(stderr, "outputting shortest-path matrix to %s...\n", filename);
    FILE *output_f = fopen(filename, "w+");
    fprintf(output_f, "%d\n", N);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            fprintf(output_f, "%.2f", C[i * N + j]);
            if (j < N - 1) {
                fputc(',', output_f);
            } else {
                fputc('\n', output_f);
            }
        }
    }
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "incorrect number of arguments\n");
        fprintf(stderr, "call as: ./main input_filename output_filename\n");
        return -1;
    }

    FILE *input_f = fopen(argv[1], "r");

    int N;  // num nodes
    if (fscanf(input_f, "%d ", &N) < 1) {
        fprintf(stderr, "malformed input: couldn't match number N of vertices\n");
        return -1;
    }

    fprintf(stderr, "allocating memory...\n");
    float *C = (float *)malloc(N * N * sizeof(float));
    fprintf(stderr, "parsing input matrix...\n");
    for (int i = 0; i < N; i++) {
        char inputValue[100];
        for (int j = 0; j < N; j++) {
            int numValues = fscanf(input_f, "%[^,\n]s", inputValue);
            if (numValues == 1) {
                C[i * N + j] = strtof(inputValue, NULL);
            } else {
                C[i * N + j] = INFINITY;
            }
            fgetc(input_f);  // skip ',' or '\n'
        }
    }
    fclose(input_f);

    float *D = (float *)malloc(N * N * sizeof(float));
    memcpy(D, C, N * N * sizeof(float));
    fprintf(stderr, "generating test output...\n");
    ref_output(D, N);
    char ref_output[256];
    sprintf(ref_output, "%s", argv[2]);
    output_matrix(ref_output, D, N);
    free(D);

#ifdef __x86_64__
    fprintf(stderr, "finding shortest paths...\n");
    double r = rdtsc(C, N);
    fprintf(stderr, "#cycles on avg: ");
    printf("%lf\n", r);
#endif

    free(C);
}