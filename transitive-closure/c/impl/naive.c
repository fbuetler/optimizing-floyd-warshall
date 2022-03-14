#include "tc.h"

/*
unoptimized version of the modified Floyd-Warshall transitive-closure algorithm
*/
void floydWarshall(char *C, int N)
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