#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include "main.h"

void floydWarshall(float *C, int N)
{
    for (int k = 0; k < N; k++)
    {
        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < N; j++)
            {
                C[i * N + j] = MIN(C[i * N + j], C[i * N + k] + C[k * N + j]);
            }
        }
    }
}

void showMatrix(float *C, int N)
{
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            printf("%f, ", C[i * N + j]);
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
    fscanf(input_f, "%d ", &N);
    printf("allocating memory...\n");
    float *C = (float *)malloc(N * N * sizeof(float));
    printf("parsing input matrix...\n");
    for (int i = 0; i < N; i++)
    {
        char inputValue[100];
        for (int j = 0; j < N; j++)
        {
            fscanf(input_f, "%[^,\n]s", inputValue);
            if (inputValue[0] != '\0')
            {
                C[i * N + j] = strtof(inputValue, NULL);
            }
            else
            {
                C[i * N + j] = INFINITY;
            }
            fgetc(input_f);
            inputValue[0] = '\0'; // ensures we don't accidentally reuse old inputValue
        }
    }
    fclose(input_f);

    printf("finding shortest paths...\n");
    floydWarshall(C, N);

    printf("outputting shortest-path matrix to %s...\n", argv[2]);
    FILE *output_f = fopen(argv[2], "w+");
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            fprintf(output_f, "%f", C[i * N + j]);
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