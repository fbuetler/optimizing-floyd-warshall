#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include "main.h"

void warshall(char *C, int N)
{
    for (int k = 0; k < N; k++)
    {
        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < N; j++)
            {
                C[i * N + j] = C[i * N + j] | (C[i * N + k] & C[k * N + j]);
            }
        }
    }
}

void showMatrix(char *C, int N)
{
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            printf("%d, ", C[i * N + j]);
        }
        printf("\n");
    }
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("incorrect number of arguments\n");
        printf("call as: ./main input_filename output_filename\n");
        return -1;
    }

    FILE *input_f = fopen(argv[1], "r");

    int N; // num nodes
    if (fscanf(input_f, "%d ", &N) < 1)
    {
        printf("malformed input: couldn't match number N of vertices\n");
        return -1;
    }

    printf("allocating memory...\n");
    char *C = (char *)malloc(N * N * sizeof(char));
    printf("parsing input matrix...\n");
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

    printf("finding transitive closure...\n");
    warshall(C, N);
    showMatrix(C, N);

    printf("outputting transitive closure matrix to %s...\n", argv[2]);
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