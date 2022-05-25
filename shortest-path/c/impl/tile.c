
#include "sp.h"

/*
// iterative FW algorithm (FWI)
// tiling factors Ui and Uj
function FWI(A, B, C, N)

    for k=1:1:N
        for i=1:Ui:N
            for j=1:Uj:N
                // loops below are completely unrolled
                for i’=i:1:i+Ui-1
                    for j’=j:1:j+Uj-1
                        C[i’][j’] = min(C[i’][j’], A[i’][k]+B[k][j’]);
*/
int floydWarshall(double *C, int N)
{
    int bsize_i = 2;
    int bsize_j = 2;

    if (N % bsize_i != 0 || N % bsize_j != 0)
    {
        return 1;
    }

    for (int k = 0; k < N; k++)
    {
        for (int i = 0; i < N; i += bsize_i)
        {
            for (int j = 0; j < N; j += bsize_j)
            {
                // tile
                for (int ii = i; ii - i < bsize_i; ii++)
                {
                    for (int jj = j; jj - j < bsize_j; jj++)
                    {
                        C[ii * N + jj] = MIN(C[ii * N + jj], C[ii * N + k] + C[k * N + jj]);
                    }
                }
            }
        }
    }
    return 0;
}
