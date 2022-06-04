#include <malloc.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <papi.h>
#include <immintrin.h>

#include "impl/tc.h"

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

// fine, I'll do it myself
void flush_cache(void *addr, int nbytes)
{
    // assume cache line of 64 bytes, cause why wouldn't it be
    void *p = addr;
    while (p < addr + nbytes)
    {
        // Invalidate and flush the cache line that contains p from all levels of the cache hierarchy
        _mm_clflush(p);
        p += 64;
    }
    // Ensure cache flushes have completed
    _mm_mfence();
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
int measure(char *C, int N, int WarmupEventSet, int MeasurementEventSet, long long *measured_values)
{
    int i, retval;
    int num_runs = NUM_RUNS;

    int bpl = ceil(N / 8.0);

    // Compute how many runs we need to do and warm up cache
    long long warmup_cycles[1];
    long long warmup_cycles_tmp[1];

    /* Start counting */
    long long cycles = 0;

    while (num_runs < (1 << 14))
    {
        warmup_cycles[0] = 0;
        for (i = 0; i < num_runs; ++i)
        {
            if ((retval = PAPI_start(WarmupEventSet)) != PAPI_OK)
            {
                ERROR_RETURN(retval);
            }

            floydWarshall(C, N);

            /* Stop counting, this reads from the counter as well as stop it. */
            if ((retval = PAPI_stop(WarmupEventSet, warmup_cycles_tmp)) != PAPI_OK)
            {
                ERROR_RETURN(retval);
            }

            warmup_cycles[0] += warmup_cycles_tmp[0]; // accumulate

            /* flush_cache((void *)C, N * bpl * sizeof(char)); */
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

    long long measured_values_tmp[NUM_EVENT];

    // ------- MEASURE CACHE MISSES ON COLD CACHE -------
    for (i = 0; i < num_runs; ++i)
    {
        /* Start counting */
        if ((retval = PAPI_start(MeasurementEventSet)) != PAPI_OK)
        {
            ERROR_RETURN(retval);
        }

        floydWarshall(C, N);

        /* Stop counting, this reads from the counter as well as stop it. */
        if ((retval = PAPI_stop(MeasurementEventSet, measured_values_tmp)) != PAPI_OK)
        {
            ERROR_RETURN(retval);
        }

        for (int i = 0; i < NUM_EVENT; i++)
        {
            measured_values[i] += measured_values_tmp[i]; // accumulate
        }

        flush_cache((void *)C, N * bpl * sizeof(char));
    }

    // ------- MEASURE NUMBER OF CYCLES ON WARM CACHE -------
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
    if ((retval = PAPI_stop(MeasurementEventSet, measured_values_tmp)) != PAPI_OK)
    {
        ERROR_RETURN(retval);
    }

    // overwrite total cycles
    measured_values[0] = measured_values_tmp[0];

    return num_runs;
}

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

/*
 * Compile with -DRANDOM_INPUT to only read N from the input files,
 * and not write anything to the output file.
 * - the input data for floyd_warshall will essentially be random bits
 */
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
    FILE *input_f = fopen(argv[1], "r");

    int N; // num nodes
    if (fscanf(input_f, "%d ", &N) < 1)
    {
        fprintf(stderr, "malformed input: couldn't match number N of vertices\n");
        return -1;
    }

    fprintf(stderr, "allocating memory...\n");
    int bpl = ceil(N / 8.0);
    char *C = (char *)aligned_alloc(32, N * bpl * sizeof(char));
    fprintf(stderr, "parsing input matrix...\n");
#ifndef RANDOM_INPUT
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
#endif
    fclose(input_f);

    char *D = (char *)aligned_alloc(32, N * bpl * sizeof(char));
    memcpy(D, C, N * bpl * sizeof(char));
    fprintf(stderr, "generating test output...\n");
    ref_output(D, N);
    char ref_output[256];
    sprintf(ref_output, "%s", argv[2]);

#ifdef RANDOM_INPUT
    sprintf(ref_output, "/dev/null");
#endif

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
