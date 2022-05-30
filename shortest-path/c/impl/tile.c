#include "sp.h"

#define SUBM(X, u, v, BL, B) (((X) + (u) * (BL) * (B) * (BL) + (v) * (BL)))

// iterative FW algorithm (FWI)
// tiling factors Ui and Uj are set to 4 and 4, respectively
int FWI(double *A, double *B, double *C, int N, int L1)
{
    // NOTE: We assume Ui and Uj divide N for simplicity
    for (int k = 0; k < L1; k++)
    {
        int i = 0;
        for (; i < L1 - 3; i += 4)
        {
            int j = 0;
            double ci0k = C[(i + 0) * N + k];
            double ci1k = C[(i + 1) * N + k];
            double ci2k = C[(i + 2) * N + k];
            double ci3k = C[(i + 3) * N + k];
            for (; j < L1 - 3; j += 4)
            {
                // load
                double ci0j0 = C[(i + 0) * N + j + 0];
                double ci0j1 = C[(i + 0) * N + j + 1];
                double ci0j2 = C[(i + 0) * N + j + 2];
                double ci0j3 = C[(i + 0) * N + j + 3];
                double ci1j0 = C[(i + 1) * N + j + 0];
                double ci1j1 = C[(i + 1) * N + j + 1];
                double ci1j2 = C[(i + 1) * N + j + 2];
                double ci1j3 = C[(i + 1) * N + j + 3];
                double ci2j0 = C[(i + 2) * N + j + 0];
                double ci2j1 = C[(i + 2) * N + j + 1];
                double ci2j2 = C[(i + 2) * N + j + 2];
                double ci2j3 = C[(i + 2) * N + j + 3];
                double ci3j0 = C[(i + 3) * N + j + 0];
                double ci3j1 = C[(i + 3) * N + j + 1];
                double ci3j2 = C[(i + 3) * N + j + 2];
                double ci3j3 = C[(i + 3) * N + j + 3];

                double ckj0 = C[k * N + j + 0];
                double ckj1 = C[k * N + j + 1];
                double ckj2 = C[k * N + j + 2];
                double ckj3 = C[k * N + j + 3];

                // op 1
                double sum0_0 = ADD(ci0k, ckj0);
                double sum0_1 = ADD(ci0k, ckj1);
                double sum0_2 = ADD(ci0k, ckj2);
                double sum0_3 = ADD(ci0k, ckj3);
                double sum1_0 = ADD(ci1k, ckj0);
                double sum1_1 = ADD(ci1k, ckj1);
                double sum1_2 = ADD(ci1k, ckj2);
                double sum1_3 = ADD(ci1k, ckj3);
                double sum2_0 = ADD(ci2k, ckj0);
                double sum2_1 = ADD(ci2k, ckj1);
                double sum2_2 = ADD(ci2k, ckj2);
                double sum2_3 = ADD(ci2k, ckj3);
                double sum3_0 = ADD(ci3k, ckj0);
                double sum3_1 = ADD(ci3k, ckj1);
                double sum3_2 = ADD(ci3k, ckj2);
                double sum3_3 = ADD(ci3k, ckj3);

                // op 2
                double res0_0 = MIN(ci0j0, sum0_0);
                double res0_1 = MIN(ci0j1, sum0_1);
                double res0_2 = MIN(ci0j2, sum0_2);
                double res0_3 = MIN(ci0j3, sum0_3);
                double res1_0 = MIN(ci1j0, sum1_0);
                double res1_1 = MIN(ci1j1, sum1_1);
                double res1_2 = MIN(ci1j2, sum1_2);
                double res1_3 = MIN(ci1j3, sum1_3);
                double res2_0 = MIN(ci2j0, sum2_0);
                double res2_1 = MIN(ci2j1, sum2_1);
                double res2_2 = MIN(ci2j2, sum2_2);
                double res2_3 = MIN(ci2j3, sum2_3);
                double res3_0 = MIN(ci3j0, sum3_0);
                double res3_1 = MIN(ci3j1, sum3_1);
                double res3_2 = MIN(ci3j2, sum3_2);
                double res3_3 = MIN(ci3j3, sum3_3);

                // store
                C[(i + 0) * N + j + 0] = res0_0;
                C[(i + 0) * N + j + 1] = res0_1;
                C[(i + 0) * N + j + 2] = res0_2;
                C[(i + 0) * N + j + 3] = res0_3;
                C[(i + 1) * N + j + 0] = res1_0;
                C[(i + 1) * N + j + 1] = res1_1;
                C[(i + 1) * N + j + 2] = res1_2;
                C[(i + 1) * N + j + 3] = res1_3;
                C[(i + 2) * N + j + 0] = res2_0;
                C[(i + 2) * N + j + 1] = res2_1;
                C[(i + 2) * N + j + 2] = res2_2;
                C[(i + 2) * N + j + 3] = res2_3;
                C[(i + 3) * N + j + 0] = res3_0;
                C[(i + 3) * N + j + 1] = res3_1;
                C[(i + 3) * N + j + 2] = res3_2;
                C[(i + 3) * N + j + 3] = res3_3;
            }
        }
    }
    return 0;
}

// FWI for 3 distinct matrices (FWIabc)
// tiling factors Uk', Ui' and Uj' are set to 2, 2 and 2, respectively
int FWIabc(double *A, double *B, double *C, int N, int L1)
{
    // NOTE: We assume Uk, Ui and Uj divide N for simplicity
    for (int i = 0; i < L1 - 1; i += 2)
    {
        for (int j = 0; j < L1 - 1; j += 2)
        {
            for (int k = 0; k < L1 - 1; k += 2)
            {
                double a00, a01, a10, a11, b00, b01, b10, b11, c00, c01, c10, c11, sum00, sum01, sum10, sum11;

                // load
                a00 = A[(i + 0) * N + (k + 0)];
                a01 = A[(i + 0) * N + (k + 1)];
                a10 = A[(i + 1) * N + (k + 0)];
                a11 = A[(i + 1) * N + (k + 1)];
                b00 = B[(k + 0) * N + (j + 0)];
                b01 = B[(k + 0) * N + (j + 1)];
                b10 = B[(k + 1) * N + (j + 0)];
                b11 = B[(k + 1) * N + (j + 1)];
                c00 = C[(i + 0) * N + (j + 0)];
                c01 = C[(i + 0) * N + (j + 1)];
                c10 = C[(i + 1) * N + (j + 0)];
                c11 = C[(i + 1) * N + (j + 1)];

                // for k + 0
                sum00 = ADD(a00, b00);
                sum01 = ADD(a00, b01);
                sum10 = ADD(a10, b00);
                sum11 = ADD(a10, b01);

                c00 = MIN(c00, sum00);
                c01 = MIN(c01, sum01);
                c10 = MIN(c10, sum10);
                c11 = MIN(c11, sum11);

                // for k + 1
                sum00 = ADD(a01, b10);
                sum01 = ADD(a01, b11);
                sum10 = ADD(a11, b10);
                sum11 = ADD(a11, b11);

                c00 = MIN(c00, sum00);
                c01 = MIN(c01, sum01);
                c10 = MIN(c10, sum10);
                c11 = MIN(c11, sum11);

                // store
                C[(i + 0) * N + (j + 0)] = c00;
                C[(i + 0) * N + (j + 1)] = c01;
                C[(i + 1) * N + (j + 0)] = c10;
                C[(i + 1) * N + (j + 1)] = c11;
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
    // tile size is set to 16
    FWT(C, C, C, N, 16);
    return 0;
}