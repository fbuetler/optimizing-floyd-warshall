#include "mm.h"

/*
unoptimized version of the modified Floyd-Warshall maximin algorithm
*/
int floydWarshall(double *C, int N)
{
    for (int k = 0; k < N; k++)
    {
        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < N; j++)
            {
                C[i * N + j] = MAX(C[i * N + j], MIN(C[i * N + k], C[k * N + j]));
            }
        }
    }
    return 0;
}