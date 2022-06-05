#include "sp.h"

#define SUBMs(X, u, v, BL, B) (((X) + (u) * (BL) * (B) * (BL) + (v) * (BL)))

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
            double a0k = A[(i + 0) * N + k];
            double a1k = A[(i + 1) * N + k];
            double a2k = A[(i + 2) * N + k];
            double a3k = A[(i + 3) * N + k];
            for (; j < L1 - 3; j += 4)
            {
                // load
                double c00 = C[(i + 0) * N + j + 0];
                double c01 = C[(i + 0) * N + j + 1];
                double c02 = C[(i + 0) * N + j + 2];
                double c03 = C[(i + 0) * N + j + 3];
                double c10 = C[(i + 1) * N + j + 0];
                double c11 = C[(i + 1) * N + j + 1];
                double c12 = C[(i + 1) * N + j + 2];
                double c13 = C[(i + 1) * N + j + 3];
                double c20 = C[(i + 2) * N + j + 0];
                double c21 = C[(i + 2) * N + j + 1];
                double c22 = C[(i + 2) * N + j + 2];
                double c23 = C[(i + 2) * N + j + 3];
                double c30 = C[(i + 3) * N + j + 0];
                double c31 = C[(i + 3) * N + j + 1];
                double c32 = C[(i + 3) * N + j + 2];
                double c33 = C[(i + 3) * N + j + 3];

                double bk0 = B[k * N + j + 0];
                double bk1 = B[k * N + j + 1];
                double bk2 = B[k * N + j + 2];
                double bk3 = B[k * N + j + 3];

                // op 1
                double sum00 = ADD(a0k, bk0);
                double sum01 = ADD(a0k, bk1);
                double sum02 = ADD(a0k, bk2);
                double sum03 = ADD(a0k, bk3);
                double sum10 = ADD(a1k, bk0);
                double sum11 = ADD(a1k, bk1);
                double sum12 = ADD(a1k, bk2);
                double sum13 = ADD(a1k, bk3);
                double sum20 = ADD(a2k, bk0);
                double sum21 = ADD(a2k, bk1);
                double sum22 = ADD(a2k, bk2);
                double sum23 = ADD(a2k, bk3);
                double sum30 = ADD(a3k, bk0);
                double sum31 = ADD(a3k, bk1);
                double sum32 = ADD(a3k, bk2);
                double sum33 = ADD(a3k, bk3);

                // op 2
                c00 = MIN(c00, sum00);
                c01 = MIN(c01, sum01);
                c02 = MIN(c02, sum02);
                c03 = MIN(c03, sum03);
                c10 = MIN(c10, sum10);
                c11 = MIN(c11, sum11);
                c12 = MIN(c12, sum12);
                c13 = MIN(c13, sum13);
                c20 = MIN(c20, sum20);
                c21 = MIN(c21, sum21);
                c22 = MIN(c22, sum22);
                c23 = MIN(c23, sum23);
                c30 = MIN(c30, sum30);
                c31 = MIN(c31, sum31);
                c32 = MIN(c32, sum32);
                c33 = MIN(c33, sum33);

                // store
                C[(i + 0) * N + j + 0] = c00;
                C[(i + 0) * N + j + 1] = c01;
                C[(i + 0) * N + j + 2] = c02;
                C[(i + 0) * N + j + 3] = c03;
                C[(i + 1) * N + j + 0] = c10;
                C[(i + 1) * N + j + 1] = c11;
                C[(i + 1) * N + j + 2] = c12;
                C[(i + 1) * N + j + 3] = c13;
                C[(i + 2) * N + j + 0] = c20;
                C[(i + 2) * N + j + 1] = c21;
                C[(i + 2) * N + j + 2] = c22;
                C[(i + 2) * N + j + 3] = c23;
                C[(i + 3) * N + j + 0] = c30;
                C[(i + 3) * N + j + 1] = c31;
                C[(i + 3) * N + j + 2] = c32;
                C[(i + 3) * N + j + 3] = c33;
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
        FWI(SUBMs(A, k, k, L1, M), SUBMs(B, k, k, L1, M), SUBMs(C, k, k, L1, M), N, L1);

        // phase 2: update all tiles in row k
        for (int j = 0; j < M; j++)
        {
            if (j != k)
            {
                // FWI(A_kk,B_kj,C_kj, L1)
                FWI(SUBMs(A, k, k, L1, M), SUBMs(B, k, j, L1, M), SUBMs(C, k, j, L1, M), N, L1);
            }
        }

        // phase 3: update all tiles in column k
        for (int i = 0; i < M; i++)
        {
            if (i != k)
            {
                // FWI(A_ik, B_kk, C_ik, L1)
                FWI(SUBMs(A, i, k, L1, M), SUBMs(B, k, k, L1, M), SUBMs(C, i, k, L1, M), N, L1);
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
                        FWIabc(SUBMs(A, i, k, L1, M), SUBMs(B, k, j, L1, M), SUBMs(C, i, j, L1, M), N, L1);
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
    return FWT(C, C, C, N, 8);
}