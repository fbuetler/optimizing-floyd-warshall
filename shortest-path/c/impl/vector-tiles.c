#include <immintrin.h>

#include "sp.h"

#define SUBM(X, u, v, BL, B) (((X) + (u) * (BL) * (B) * (BL) + (v) * (BL)))

// iterative FW algorithm (FWI)
// tiling factors Ui and Uj are set to 4 and 4, respectively
int FWI(double *A, double *B, double *C, int N, int L1)
{
    // NOTE: We assume Ui and Uj divide N for simplicity
    for (int k = 0; k < L1; k++)
    {
        for (int i = 0; i < L1 - 3; i += 4)
        {
            __m256d a0k = _mm256_broadcast_sd(&A[(i + 0) * N + k]);
            __m256d a1k = _mm256_broadcast_sd(&A[(i + 1) * N + k]);
            __m256d a2k = _mm256_broadcast_sd(&A[(i + 2) * N + k]);
            __m256d a3k = _mm256_broadcast_sd(&A[(i + 3) * N + k]);
            for (int j = 0; j < L1 - 3; j += 4)
            {
                // load
                __m256d c0j = _mm256_load_pd(&C[(i + 0) * N + j]);
                __m256d c1j = _mm256_load_pd(&C[(i + 1) * N + j]);
                __m256d c2j = _mm256_load_pd(&C[(i + 2) * N + j]);
                __m256d c3j = _mm256_load_pd(&C[(i + 3) * N + j]);

                __m256d bkj = _mm256_load_pd(&B[k * N + j]);

                // op 1
                __m256d sum0 = _mm256_add_pd(a0k, bkj);
                __m256d sum1 = _mm256_add_pd(a1k, bkj);
                __m256d sum2 = _mm256_add_pd(a2k, bkj);
                __m256d sum3 = _mm256_add_pd(a3k, bkj);

                // op 2
                c0j = _mm256_min_pd(c0j, sum0);
                c1j = _mm256_min_pd(c1j, sum1);
                c2j = _mm256_min_pd(c2j, sum2);
                c3j = _mm256_min_pd(c3j, sum3);

                // store
                _mm256_store_pd(&C[(i + 0) * N + j], c0j);
                _mm256_store_pd(&C[(i + 1) * N + j], c1j);
                _mm256_store_pd(&C[(i + 2) * N + j], c2j);
                _mm256_store_pd(&C[(i + 3) * N + j], c3j);
            }
        }
    }
    return 0;
}

// FWI for 3 distinct matrices (FWIabc)
// tiling factors Uk', Ui' and Uj' are set to 2, 4 and 4, respectively
int FWIabc(double *A, double *B, double *C, int N, int L1)
{
    // NOTE: We assume Uk, Ui and Uj divide N for simplicity
    for (int i = 0; i < L1 - 3; i += 4)
    {
        for (int j = 0; j < L1 - 3; j += 4)
        {
            for (int k = 0; k < L1 - 1; k += 2)
            {
                __m256d a00, a01, a10, a11, a20, a21, a30, a31, b0j, b1j, c0j, c1j, c2j, c3j, sum0, sum1, sum2, sum3;

                // load
                a00 = _mm256_broadcast_sd(&A[(i + 0) * N + (k + 0)]);
                a01 = _mm256_broadcast_sd(&A[(i + 0) * N + (k + 1)]);
                a10 = _mm256_broadcast_sd(&A[(i + 1) * N + (k + 0)]);
                a11 = _mm256_broadcast_sd(&A[(i + 1) * N + (k + 1)]);
                a20 = _mm256_broadcast_sd(&A[(i + 2) * N + (k + 0)]);
                a21 = _mm256_broadcast_sd(&A[(i + 2) * N + (k + 1)]);
                a30 = _mm256_broadcast_sd(&A[(i + 3) * N + (k + 0)]);
                a31 = _mm256_broadcast_sd(&A[(i + 3) * N + (k + 1)]);
                b0j = _mm256_load_pd(&B[(k + 0) * N + j]);
                b1j = _mm256_load_pd(&B[(k + 1) * N + j]);
                c0j = _mm256_load_pd(&C[(i + 0) * N + j]);
                c1j = _mm256_load_pd(&C[(i + 1) * N + j]);
                c2j = _mm256_load_pd(&C[(i + 2) * N + j]);
                c3j = _mm256_load_pd(&C[(i + 3) * N + j]);

                // for k + 0
                sum0 = _mm256_add_pd(a00, b0j);
                sum1 = _mm256_add_pd(a10, b0j);
                sum2 = _mm256_add_pd(a20, b0j);
                sum3 = _mm256_add_pd(a30, b0j);

                c0j = _mm256_min_pd(c0j, sum0);
                c1j = _mm256_min_pd(c1j, sum1);
                c2j = _mm256_min_pd(c2j, sum2);
                c3j = _mm256_min_pd(c3j, sum3);

                // for k + 1
                sum0 = _mm256_add_pd(a01, b1j);
                sum1 = _mm256_add_pd(a11, b1j);
                sum2 = _mm256_add_pd(a21, b1j);
                sum3 = _mm256_add_pd(a31, b1j);

                c0j = _mm256_min_pd(c0j, sum0);
                c1j = _mm256_min_pd(c1j, sum1);
                c2j = _mm256_min_pd(c2j, sum2);
                c3j = _mm256_min_pd(c3j, sum3);

                // store
                _mm256_store_pd(&C[(i + 0) * N + j], c0j);
                _mm256_store_pd(&C[(i + 1) * N + j], c1j);
                _mm256_store_pd(&C[(i + 2) * N + j], c2j);
                _mm256_store_pd(&C[(i + 3) * N + j], c3j);
            }
        }
    }
    return 0;
}

// tiled FW algorithm (FWT)
// tile size: L1 x L1
int FWT(double *A, double *B, double *C, int N, int L1)
{
    // NOTE: We assume L1 divides N for simplicity
    // Notation: A_ij is the L1 x L1 submatrix (i,j) of A
    int M = N / L1;
    for (int k = 0; k < M; k++)
    {
        // phase 1: update all diagonal tiles
        // FWI(A_kk, B_kk, C_kk, L1)
        FWI(SUBM(A, k, k, L1, M), SUBM(B, k, k, L1, M), SUBM(C, k, k, L1, M), N, L1);

        // phase 2: update all tiles in row k
        for (int j = 0; j < M; j++)
        {
            if (j != k)
            {
                // FWI(A_kk,B_kj,C_kj, L1)
                FWI(SUBM(A, k, k, L1, M), SUBM(B, k, j, L1, M), SUBM(C, k, j, L1, M), N, L1);
            }
        }

        // phase 3: update all tiles in column k
        for (int i = 0; i < M; i++)
        {
            if (i != k)
            {
                // FWI(A_ik, B_kk, C_ik, L1)
                FWI(SUBM(A, i, k, L1, M), SUBM(B, k, k, L1, M), SUBM(C, i, k, L1, M), N, L1);
            }
        }

        // phase 4: update all remaining tiles
        for (int i = 0; i < M; i++)
        {
            if (i != k)
            {
                for (int j = 0; j < M; j++)
                {
                    if (j != k)
                    {
                        // FWIabc(A_ik,B_kj,C_ij, L1)
                        FWIabc(SUBM(A, i, k, L1, M), SUBM(B, k, j, L1, M), SUBM(C, i, j, L1, M), N, L1);
                    }
                }
            }
        }
    }
    return 0;
}

/*
// tiled FW algorithm (FWT)
// tile size: L1 x L1
function FWT(A, B, C, N, L1)
    // A_ij: L1 x L1 submatrix (i,j) of A, i.e., A[(i-1)*L1+1:i*L1][(j-1)*L1+1:j*L1];
    M = N/L1;
    for k=1:1:M
        // phase 1
        FWI(A_kk, B_kk, C_kk, L1);
        // phase 2
        for j=1:1:M, j!=k
            FWI(A_kk,B_kj,C_kj, L1);
        // phase 3
        for i=1:1:M, i!=k
            FWI(A_ik, B_kk, C_ik, L1);
        // phase 4
        for i=1:1:M, i!=k
            for j=1:1:M, j!=k
                FWIabc(A_ik,B_kj,C_ij, L1);

*/
int floydWarshall(double *C, int N)
{
    // tile size is set to 8
    return FWT(C, C, C, N, 32);
}