#include <malloc.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <papi.h>

#include "impl/sp.h"

#define NUM_RUNS 16
#define CYCLES_REQUIRED 1e8
#define FREQUENCY 2.3e9
#define CALIBRATE

#define ERROR_RETURN(retval)                                                         \
    {                                                                                \
        fprintf(stderr, "PAPI error %d %s:line %d: \n", retval, __FILE__, __LINE__); \
        exit(retval);                                                                \
    }

// The number of PAPI events we measure
#define NUM_EVENT 4

// The PAPI events we subscribe to for the measurement
// Only add events supported on your machine. Check with the papi_avail tool in the PAPI utils
// IMPORTANT: DO NOT CHANGE ORDER! Otherwise the data readout in the measurement script breaks
static int event_codes[NUM_EVENT] = {
    PAPI_TOT_CYC, // Total cycles executed
    PAPI_L3_TCM,  // L3 cache misses
    PAPI_L2_TCM,  // L2 data cache misses
    PAPI_L1_DCM,  // L1 data cache misses
};

void printMatrix(float *C, int N)
{
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            fprintf(stderr, "%.2f, ", C[i * N + j]);
        }
        fprintf(stderr, "\n");
    }
}

/*
Copies data from one matrix to another.
Assumes matrices 'from' and 'to' point to separate memory locations.
*/
void copyMatrix(float *from, float *to, int N)
{
#pragma ivdep
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            to[i * N + j] = from[i * N + j];
        }
    }
}

/*
 * Runs the FW implementation once for testing purposes
 * Note that the matrix C is modified in-place
 */
void ref_output(double *C, int N)
{
    int err = floydWarshall(C, N);
    if (err != EXIT_SUCCESS)
    {
        printf("implementation reported an error\n");
        exit(EXIT_FAILURE);
    }
}

/*
 * Timing function based on the TimeStep Counter of the CPU.
 *
 * We don't particularly care about the input, so we just
 * run FW on the same matrix C for each run even though it
 * modifies it in-place.
 *
 * The function returns the average number of cycles per run.
 */
int measure(double *C, int N, int WarmupEventSet, int MeasurementEventSet, long long *measured_values)
{
    int i, retval;
    int num_runs = NUM_RUNS;

    // Compute how many runs we need to do and warm up cache
    long long warmup_cycles[1];
    /* Start counting */
    long long cycles = 0;
    while (num_runs < (1 << 14))
    {
        if ((retval = PAPI_start(WarmupEventSet)) != PAPI_OK)
        {
            ERROR_RETURN(retval);
        }

        for (i = 0; i < num_runs; ++i)
        {
            floydWarshall(C, N);
        }

        /* Stop counting, this reads from the counter as well as stop it. */
        if ((retval = PAPI_stop(WarmupEventSet, warmup_cycles)) != PAPI_OK)
        {
            ERROR_RETURN(retval);
        }

        cycles += warmup_cycles[0];

        if (cycles >= CYCLES_REQUIRED)
        {
            break;
        }

        num_runs *= 2;
        if ((retval = PAPI_reset(WarmupEventSet)) != PAPI_OK)
        {
            ERROR_RETURN(retval);
        }
    }

    fprintf(stderr, "#runs: %d\n", num_runs);
    /*
     * Alternatives to the current approach:
     * - Measure #cycles for each run separately
     *  --> Suffers from timing bias and requires cache warmup
     *  --> Allows for more fine-grained statistics, e.g. median, variance, etc.
     */
    /* Start counting */
    if ((retval = PAPI_start(MeasurementEventSet)) != PAPI_OK)
    {
        ERROR_RETURN(retval);
    }

    for (i = 0; i < num_runs; ++i)
    {
        floydWarshall(C, N);
    }

    /* Stop counting, this reads from the counter as well as stop it. */
    if ((retval = PAPI_stop(MeasurementEventSet, measured_values)) != PAPI_OK)
    {
        ERROR_RETURN(retval);
    }

    return num_runs;
}

void output_matrix(char *filename, double *C, int N)
{
    fprintf(stderr, "outputting shortest-path matrix to %s...\n", filename);
    FILE *output_f = fopen(filename, "w+");
    fprintf(output_f, "%d\n", N);

    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            fprintf(output_f, "%.2f", C[i * N + j]);
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

    // setup PAPI
    int retval;
    int WarmupEventSet = PAPI_NULL;
    int MeasurementEventSet = PAPI_NULL;
    char errstring[PAPI_MAX_STR_LEN];
    long long measured_values[NUM_EVENT];

    if ((retval = PAPI_library_init(PAPI_VER_CURRENT)) != PAPI_VER_CURRENT)
    {
        fprintf(stderr, "Error: %s\n", errstring);
        return 1;
    }

    /* Creating event set   */
    if ((retval = PAPI_create_eventset(&MeasurementEventSet)) != PAPI_OK)
    {
        ERROR_RETURN(retval);
    }
    if ((retval = PAPI_create_eventset(&WarmupEventSet)) != PAPI_OK)
    {
        ERROR_RETURN(retval);
    }

    // add events to event sets
    if ((retval = PAPI_add_event(WarmupEventSet, PAPI_TOT_CYC)) != PAPI_OK)
    {
        ERROR_RETURN(retval);
    }

    if ((retval = PAPI_add_events(MeasurementEventSet, event_codes, NUM_EVENT)) != PAPI_OK)
    {
        ERROR_RETURN(retval);
    }

    // read input
    char *input_fname = argv[1];
    char *output_fname = argv[2];

    FILE *input_f = fopen(input_fname, "r");
    int N; // num nodes
    if (fscanf(input_f, "%d ", &N) < 1)
    {
        fprintf(stderr, "malformed input: couldn't match number N of vertices\n");
        return -1;
    }

    fprintf(stderr, "allocating memory...\n");
    double *C = (double *)aligned_alloc(32, N * N * sizeof(double));
    fprintf(stderr, "parsing input matrix...\n");
    for (int i = 0; i < N; i++)
    {
        char inputValue[100];
        for (int j = 0; j < N; j++)
        {
            int numValues = fscanf(input_f, "%[^,\n]s", inputValue);
            if (numValues == 1)
            {
                C[i * N + j] = strtof(inputValue, NULL);
            }
            else
            {
                C[i * N + j] = INFINITY;
            }
            fgetc(input_f); // skip ',' or '\n'
        }
    }
    fclose(input_f);

    remove(output_fname);
    double *D = (double *)aligned_alloc(32, N * N * sizeof(double));
    memcpy(D, C, N * N * sizeof(double));
    fprintf(stderr, "generating test output...\n");
    ref_output(D, N);
    char ref_output[256];
    sprintf(ref_output, "%s", output_fname);
    output_matrix(ref_output, D, N);
    free(D);

    // run measurements
    fprintf(stderr, "measuring shortest paths for n=%d\n", N);
    int num_runs = measure(C, N, WarmupEventSet, MeasurementEventSet, measured_values);

    // output measurements
    printf("%d", num_runs);
    for (int i = 0; i < NUM_EVENT; i++)
    {
        printf("\n%lld", measured_values[i] / num_runs);
    }

    // clean up
    free(C);

    /* Free all memory and data structures, EventSet must be empty. */
    if ((retval = PAPI_cleanup_eventset(WarmupEventSet)) != PAPI_OK)
    {
        ERROR_RETURN(retval);
    }
    if ((retval = PAPI_cleanup_eventset(MeasurementEventSet)) != PAPI_OK)
    {
        ERROR_RETURN(retval);
    }

    if ((retval = PAPI_destroy_eventset(&WarmupEventSet)) != PAPI_OK)
    {
        ERROR_RETURN(retval);
    }
    if ((retval = PAPI_destroy_eventset(&MeasurementEventSet)) != PAPI_OK)
    {
        ERROR_RETURN(retval);
    }

    /* free the resources used by PAPI */
    PAPI_shutdown();

    return EXIT_SUCCESS;
}