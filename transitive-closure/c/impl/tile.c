#include "tc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO: Make bpt an actual argument you lazy fuck
#define SUBM(X, u, v, BL, byl, byt) (((X) + (u) * (byl) * (BL) + (v) * (byt)))

// iterative FW algorithm (FWI)
// tiling factors Ui and Uj are set to 4 and 4, respectively
int FWI(char *A, char *B, char *C, int N, int L1)
{
    int bpl = ceil(N / 8.0);  // bytes per matrix line
    int bpt = ceil(L1 / 8.0); // bytes per tile line

    // NOTE: We assume Ui and Uj divide N for simplicity
    for (int k = 0; k < L1; k++)
    {
        for (int i = 0; i < L1 - 3; i += 4)
        {
            char a0k = A[(i + 0) * bpl + k / 8] & (1 << (k % 8)) ? 0xff : 0x00; // if this is 0, we can skip the innermost loop
            char a1k = A[(i + 1) * bpl + k / 8] & (1 << (k % 8)) ? 0xff : 0x00; // if this is 0, we can skip the innermost loop
            char a2k = A[(i + 2) * bpl + k / 8] & (1 << (k % 8)) ? 0xff : 0x00; // if this is 0, we can skip the innermost loop
            char a3k = A[(i + 3) * bpl + k / 8] & (1 << (k % 8)) ? 0xff : 0x00; // if this is 0, we can skip the innermost loop
            for (int j = 0; j < bpt - 3; j += 4)
            {
                // load
                char c00 = C[(i + 0) * bpl + j + 0];
                char c01 = C[(i + 0) * bpl + j + 1];
                char c02 = C[(i + 0) * bpl + j + 2];
                char c03 = C[(i + 0) * bpl + j + 3];
                char c10 = C[(i + 1) * bpl + j + 0];
                char c11 = C[(i + 1) * bpl + j + 1];
                char c12 = C[(i + 1) * bpl + j + 2];
                char c13 = C[(i + 1) * bpl + j + 3];
                char c20 = C[(i + 2) * bpl + j + 0];
                char c21 = C[(i + 2) * bpl + j + 1];
                char c22 = C[(i + 2) * bpl + j + 2];
                char c23 = C[(i + 2) * bpl + j + 3];
                char c30 = C[(i + 3) * bpl + j + 0];
                char c31 = C[(i + 3) * bpl + j + 1];
                char c32 = C[(i + 3) * bpl + j + 2];
                char c33 = C[(i + 3) * bpl + j + 3];

                char bk0 = B[k * bpl + j + 0];
                char bk1 = B[k * bpl + j + 1];
                char bk2 = B[k * bpl + j + 2];
                char bk3 = B[k * bpl + j + 3];

                // op 1
                char sum00 = AND(a0k, bk0);
                char sum01 = AND(a0k, bk1);
                char sum02 = AND(a0k, bk2);
                char sum03 = AND(a0k, bk3);
                char sum10 = AND(a1k, bk0);
                char sum11 = AND(a1k, bk1);
                char sum12 = AND(a1k, bk2);
                char sum13 = AND(a1k, bk3);
                char sum20 = AND(a2k, bk0);
                char sum21 = AND(a2k, bk1);
                char sum22 = AND(a2k, bk2);
                char sum23 = AND(a2k, bk3);
                char sum30 = AND(a3k, bk0);
                char sum31 = AND(a3k, bk1);
                char sum32 = AND(a3k, bk2);
                char sum33 = AND(a3k, bk3);

                // op 2
                c00 = OR(c00, sum00);
                c01 = OR(c01, sum01);
                c02 = OR(c02, sum02);
                c03 = OR(c03, sum03);
                c10 = OR(c10, sum10);
                c11 = OR(c11, sum11);
                c12 = OR(c12, sum12);
                c13 = OR(c13, sum13);
                c20 = OR(c20, sum20);
                c21 = OR(c21, sum21);
                c22 = OR(c22, sum22);
                c23 = OR(c23, sum23);
                c30 = OR(c30, sum30);
                c31 = OR(c31, sum31);
                c32 = OR(c32, sum32);
                c33 = OR(c33, sum33);

                // store
                C[(i + 0) * bpl + j + 0] = c00;
                C[(i + 0) * bpl + j + 1] = c01;
                C[(i + 0) * bpl + j + 2] = c02;
                C[(i + 0) * bpl + j + 3] = c03;
                C[(i + 1) * bpl + j + 0] = c10;
                C[(i + 1) * bpl + j + 1] = c11;
                C[(i + 1) * bpl + j + 2] = c12;
                C[(i + 1) * bpl + j + 3] = c13;
                C[(i + 2) * bpl + j + 0] = c20;
                C[(i + 2) * bpl + j + 1] = c21;
                C[(i + 2) * bpl + j + 2] = c22;
                C[(i + 2) * bpl + j + 3] = c23;
                C[(i + 3) * bpl + j + 0] = c30;
                C[(i + 3) * bpl + j + 1] = c31;
                C[(i + 3) * bpl + j + 2] = c32;
                C[(i + 3) * bpl + j + 3] = c33;
            }
        }
    }
    return 0;
}

// FWI for 3 distinct matrices (FWIabc)
// tiling factors Uk', Ui' and Uj' are set to 2, 2 and 2, respectively
int FWIabc(char *A, char *B, char *C, int N, int L1)
{
    int bpl = ceil(N / 8.0);  // bytes per matrix line
    int bpt = ceil(L1 / 8.0); // bytes per tile line

    // NOTE: We assume Uk, Ui and Uj divide N for simplicity
    for (int i = 0; i < L1 - 1; i += 2)
    {
        for (int j = 0; j < bpt - 1; j += 2)
        {
            for (int k = 0; k < L1 - 1; k += 2)
            {
                char a00, a01, a10, a11, b00, b01, b10, b11, c00, c01, c10, c11, sum00, sum01, sum10, sum11;

                // load
                a00 = A[(i + 0) * bpl + (k / 8 + 0)];
                a01 = A[(i + 0) * bpl + (k / 8 + 1)];
                a10 = A[(i + 1) * bpl + (k / 8 + 0)];
                a11 = A[(i + 1) * bpl + (k / 8 + 1)];
                b00 = B[(k + 0) * bpl + (j + 0)];
                b01 = B[(k + 0) * bpl + (j + 1)];
                b10 = B[(k + 1) * bpl + (j + 0)];
                b11 = B[(k + 1) * bpl + (j + 1)];
                c00 = C[(i + 0) * bpl + (j + 0)];
                c01 = C[(i + 0) * bpl + (j + 1)];
                c10 = C[(i + 1) * bpl + (j + 0)];
                c11 = C[(i + 1) * bpl + (j + 1)];

                // for k + 0
                sum00 = AND(a00, b00);
                sum01 = AND(a00, b01);
                sum10 = AND(a10, b00);
                sum11 = AND(a10, b01);

                c00 = OR(c00, sum00);
                c01 = OR(c01, sum01);
                c10 = OR(c10, sum10);
                c11 = OR(c11, sum11);

                // for k + 1
                sum00 = AND(a01, b10);
                sum01 = AND(a01, b11);
                sum10 = AND(a11, b10);
                sum11 = AND(a11, b11);

                c00 = OR(c00, sum00);
                c01 = OR(c01, sum01);
                c10 = OR(c10, sum10);
                c11 = OR(c11, sum11);

                // store
                C[(i + 0) * bpl + (j + 0)] = c00;
                C[(i + 0) * bpl + (j + 1)] = c01;
                C[(i + 1) * bpl + (j + 0)] = c10;
                C[(i + 1) * bpl + (j + 1)] = c11;
            }
        }
    }
    return 0;
}

// tiled FW algorithm (FWT)
// tile size: L1 x L1
int FWT(char *A, char *B, char *C, int N, int L1)
{
    int bpl = ceil(N / 8.0);  // bytes per matrix line
    int bpt = ceil(L1 / 8.0); // bytes per tile line

    // NOTE: We assume L1 divides N for simplicity
    // Notation: A_ij is the L1 x L1 submatrix (i,j) of A
    int M = N / L1;
    for (int k = 0; k < M; k++)
    {
        // phase 1: update all diagonal tiles
        // FWI(A_kk, B_kk, C_kk, L1)
        FWI(SUBM(A, k, k, L1, bpl, bpt), SUBM(B, k, k, L1, bpl, bpt), SUBM(C, k, k, L1, bpl, bpt), N, L1);

        // phase 2: update all tiles in row k
        for (int j = 0; j < M; j++)
        {
            if (j != k)
            {
                // FWI(A_kk,B_kj,C_kj, L1)
                FWI(SUBM(A, k, k, L1, bpl, bpt), SUBM(B, k, j, L1, bpl, bpt), SUBM(C, k, j, L1, bpl, bpt), N, L1);
            }
        }

        // phase 3: update all tiles in column k
        for (int i = 0; i < M; i++)
        {
            if (i != k)
            {
                // FWI(A_ik, B_kk, C_ik, L1)
                FWI(SUBM(A, i, k, L1, bpl, bpt), SUBM(B, k, k, L1, bpl, bpt), SUBM(C, i, k, L1, bpl, bpt), N, L1);
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
                        FWIabc(SUBM(A, i, k, L1, bpl, bpt), SUBM(B, k, j, L1, bpl, bpt), SUBM(C, i, j, L1, bpl, bpt), N, L1);
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
int floydWarshall(char *C, int N)
{
    // tile size is set to 32
    // (this is the minimum due to Uj | L1 | N assumption)
    return FWT(C, C, C, N, 32);
}