#include <immintrin.h>

#include "sp.h"

// iterative FW algorithm (FWI)
// tiling factors Ui and Uj are set to 16 and 4, respectively
int FWI(double *A, double *B, double *C, int N, int L1)
{
    // NOTE 1: We assume Ui and Uj divide N for simplicity
    // NOTE 2: If generating vector code, we require that Uj is divisible by 4
    for (int k = 0; k < L1; k++)
    {
        for (int i = 0; i < L1 - 15; i += 16)
        {
            __m256d a_0_k = _mm256_broadcast_sd(&A[(i + 0) * N + k]);
            __m256d a_1_k = _mm256_broadcast_sd(&A[(i + 1) * N + k]);
            __m256d a_2_k = _mm256_broadcast_sd(&A[(i + 2) * N + k]);
            __m256d a_3_k = _mm256_broadcast_sd(&A[(i + 3) * N + k]);
            __m256d a_4_k = _mm256_broadcast_sd(&A[(i + 4) * N + k]);
            __m256d a_5_k = _mm256_broadcast_sd(&A[(i + 5) * N + k]);
            __m256d a_6_k = _mm256_broadcast_sd(&A[(i + 6) * N + k]);
            __m256d a_7_k = _mm256_broadcast_sd(&A[(i + 7) * N + k]);
            __m256d a_8_k = _mm256_broadcast_sd(&A[(i + 8) * N + k]);
            __m256d a_9_k = _mm256_broadcast_sd(&A[(i + 9) * N + k]);
            __m256d a_10_k = _mm256_broadcast_sd(&A[(i + 10) * N + k]);
            __m256d a_11_k = _mm256_broadcast_sd(&A[(i + 11) * N + k]);
            __m256d a_12_k = _mm256_broadcast_sd(&A[(i + 12) * N + k]);
            __m256d a_13_k = _mm256_broadcast_sd(&A[(i + 13) * N + k]);
            __m256d a_14_k = _mm256_broadcast_sd(&A[(i + 14) * N + k]);
            __m256d a_15_k = _mm256_broadcast_sd(&A[(i + 15) * N + k]);
            for (int j = 0; j < L1 - 3; j += 4)
            {
                // load
                __m256d c_0_0 = _mm256_load_pd(&C[(i + 0) * N + j + 0]);
                __m256d c_1_0 = _mm256_load_pd(&C[(i + 1) * N + j + 0]);
                __m256d c_2_0 = _mm256_load_pd(&C[(i + 2) * N + j + 0]);
                __m256d c_3_0 = _mm256_load_pd(&C[(i + 3) * N + j + 0]);
                __m256d c_4_0 = _mm256_load_pd(&C[(i + 4) * N + j + 0]);
                __m256d c_5_0 = _mm256_load_pd(&C[(i + 5) * N + j + 0]);
                __m256d c_6_0 = _mm256_load_pd(&C[(i + 6) * N + j + 0]);
                __m256d c_7_0 = _mm256_load_pd(&C[(i + 7) * N + j + 0]);
                __m256d c_8_0 = _mm256_load_pd(&C[(i + 8) * N + j + 0]);
                __m256d c_9_0 = _mm256_load_pd(&C[(i + 9) * N + j + 0]);
                __m256d c_10_0 = _mm256_load_pd(&C[(i + 10) * N + j + 0]);
                __m256d c_11_0 = _mm256_load_pd(&C[(i + 11) * N + j + 0]);
                __m256d c_12_0 = _mm256_load_pd(&C[(i + 12) * N + j + 0]);
                __m256d c_13_0 = _mm256_load_pd(&C[(i + 13) * N + j + 0]);
                __m256d c_14_0 = _mm256_load_pd(&C[(i + 14) * N + j + 0]);
                __m256d c_15_0 = _mm256_load_pd(&C[(i + 15) * N + j + 0]);
                

                __m256d b_k_0 = _mm256_load_pd(&B[k * N + j + 0]);
                

                // op 1
                __m256d res_0_0 = VADD(a_0_k, b_k_0);
                __m256d res_1_0 = VADD(a_1_k, b_k_0);
                __m256d res_2_0 = VADD(a_2_k, b_k_0);
                __m256d res_3_0 = VADD(a_3_k, b_k_0);
                __m256d res_4_0 = VADD(a_4_k, b_k_0);
                __m256d res_5_0 = VADD(a_5_k, b_k_0);
                __m256d res_6_0 = VADD(a_6_k, b_k_0);
                __m256d res_7_0 = VADD(a_7_k, b_k_0);
                __m256d res_8_0 = VADD(a_8_k, b_k_0);
                __m256d res_9_0 = VADD(a_9_k, b_k_0);
                __m256d res_10_0 = VADD(a_10_k, b_k_0);
                __m256d res_11_0 = VADD(a_11_k, b_k_0);
                __m256d res_12_0 = VADD(a_12_k, b_k_0);
                __m256d res_13_0 = VADD(a_13_k, b_k_0);
                __m256d res_14_0 = VADD(a_14_k, b_k_0);
                __m256d res_15_0 = VADD(a_15_k, b_k_0);
                

                // op 2
                c_0_0 = VMIN(c_0_0, res_0_0);
                c_1_0 = VMIN(c_1_0, res_1_0);
                c_2_0 = VMIN(c_2_0, res_2_0);
                c_3_0 = VMIN(c_3_0, res_3_0);
                c_4_0 = VMIN(c_4_0, res_4_0);
                c_5_0 = VMIN(c_5_0, res_5_0);
                c_6_0 = VMIN(c_6_0, res_6_0);
                c_7_0 = VMIN(c_7_0, res_7_0);
                c_8_0 = VMIN(c_8_0, res_8_0);
                c_9_0 = VMIN(c_9_0, res_9_0);
                c_10_0 = VMIN(c_10_0, res_10_0);
                c_11_0 = VMIN(c_11_0, res_11_0);
                c_12_0 = VMIN(c_12_0, res_12_0);
                c_13_0 = VMIN(c_13_0, res_13_0);
                c_14_0 = VMIN(c_14_0, res_14_0);
                c_15_0 = VMIN(c_15_0, res_15_0);
                

                // store
                _mm256_store_pd(&C[(i + 0) * N + j + 0], c_0_0);
                _mm256_store_pd(&C[(i + 1) * N + j + 0], c_1_0);
                _mm256_store_pd(&C[(i + 2) * N + j + 0], c_2_0);
                _mm256_store_pd(&C[(i + 3) * N + j + 0], c_3_0);
                _mm256_store_pd(&C[(i + 4) * N + j + 0], c_4_0);
                _mm256_store_pd(&C[(i + 5) * N + j + 0], c_5_0);
                _mm256_store_pd(&C[(i + 6) * N + j + 0], c_6_0);
                _mm256_store_pd(&C[(i + 7) * N + j + 0], c_7_0);
                _mm256_store_pd(&C[(i + 8) * N + j + 0], c_8_0);
                _mm256_store_pd(&C[(i + 9) * N + j + 0], c_9_0);
                _mm256_store_pd(&C[(i + 10) * N + j + 0], c_10_0);
                _mm256_store_pd(&C[(i + 11) * N + j + 0], c_11_0);
                _mm256_store_pd(&C[(i + 12) * N + j + 0], c_12_0);
                _mm256_store_pd(&C[(i + 13) * N + j + 0], c_13_0);
                _mm256_store_pd(&C[(i + 14) * N + j + 0], c_14_0);
                _mm256_store_pd(&C[(i + 15) * N + j + 0], c_15_0);
                
            }
        }
    }
    return 0;
}

// FWI for 3 distinct matrices (FWIabc)
// tiling factors Uk', Ui' and Uj' are set to 1, 1 and 4, respectively
int FWIabc(double *A, double *B, double *C, int N, int L1)
{
    // NOTE: We assume Uk, Ui and Uj divide N for simplicity
    for (int i = 0; i < L1 - 0; i += 1)
    {
        for (int j = 0; j < L1 - 3; j += 4)
        {
            for (int k = 0; k < L1 - 0; k += 1)
            {
                // load
                __m256d a_0_0 = _mm256_broadcast_sd(&A[(i + 0) * N + (k + 0)]);
                

                __m256d b_0_0 = _mm256_load_pd(&B[(k + 0) * N + (j + 0)]);
                

                __m256d c_0_0 = _mm256_load_pd(&C[(i + 0) * N + j + 0]);
                

                // for k + 0
                __m256d res_0_0 = VADD(a_0_0, b_0_0);
                

                c_0_0 = VMIN(c_0_0, res_0_0);
                
                // store
                _mm256_store_pd(&C[(i + 0) * N + j + 0], c_0_0);
                
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
        FWI(SUBM(A, k, k, N, L1, L1), SUBM(B, k, k, N, L1, L1), SUBM(C, k, k, N, L1, L1), N, L1);

        // phase 2: update all tiles in row k
        for (int j = 0; j < M; j++)
        {
            if (j != k)
            {
                // FWI(A_kk,B_kj,C_kj, L1)
                FWI(SUBM(A, k, k, N, L1, L1), SUBM(B, k, j, N, L1, L1), SUBM(C, k, j, N, L1, L1), N, L1);
            }
        }

        // phase 3: update all tiles in column k
        for (int i = 0; i < M; i++)
        {
            if (i != k)
            {
                // FWI(A_ik, B_kk, C_ik, L1)
                FWI(SUBM(A, i, k, N, L1, L1), SUBM(B, k, k, N, L1, L1), SUBM(C, i, k, N, L1, L1), N, L1);
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
                        FWIabc(SUBM(A, i, k, N, L1, L1), SUBM(B, k, j, N, L1, L1), SUBM(C, i, j, N, L1, L1), N, L1);
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
    return FWT(C, C, C, N, 64);
}