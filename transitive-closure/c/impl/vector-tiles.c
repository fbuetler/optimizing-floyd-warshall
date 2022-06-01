#include "tc.h"
#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO: Make bpt an actual argument you lazy fuck
#define SUBM(X, u, v, BL, byl, byt) (((X) + (u) * (byl) * (BL) + (v) * (byt)))
const int bpv = 256 / 8; // vectors per tile line

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
            char a0k = A[(i + 0) * bpl + k / 8] & (1 << (k % 8)) ? 0xff : 0x00;
            char a1k = A[(i + 1) * bpl + k / 8] & (1 << (k % 8)) ? 0xff : 0x00;
            char a2k = A[(i + 2) * bpl + k / 8] & (1 << (k % 8)) ? 0xff : 0x00;
            char a3k = A[(i + 3) * bpl + k / 8] & (1 << (k % 8)) ? 0xff : 0x00;
            __m256i va0k = _mm256_set1_epi8(a0k);
            __m256i va1k = _mm256_set1_epi8(a1k);
            __m256i va2k = _mm256_set1_epi8(a2k);
            __m256i va3k = _mm256_set1_epi8(a3k);
            for (int j = 0; j < bpt - (bpv - 1); j += bpv)
            {
                // load
                __m256i vc0 = _mm256_loadu_si256((__m256i const *)&C[(i + 0) * bpl + j]);
                __m256i vc1 = _mm256_loadu_si256((__m256i const *)&C[(i + 1) * bpl + j]);
                __m256i vc2 = _mm256_loadu_si256((__m256i const *)&C[(i + 2) * bpl + j]);
                __m256i vc3 = _mm256_loadu_si256((__m256i const *)&C[(i + 3) * bpl + j]);

                __m256i vbk = _mm256_loadu_si256((__m256i const *)&B[k * bpl + j]);

                // op 1
                __m256i sum0 = _mm256_and_si256(va0k, vbk);
                __m256i sum1 = _mm256_and_si256(va1k, vbk);
                __m256i sum2 = _mm256_and_si256(va2k, vbk);
                __m256i sum3 = _mm256_and_si256(va3k, vbk);

                // op 2
                __m256i vres0 = _mm256_or_si256(vc0, sum0);
                __m256i vres1 = _mm256_or_si256(vc1, sum1);
                __m256i vres2 = _mm256_or_si256(vc2, sum2);
                __m256i vres3 = _mm256_or_si256(vc3, sum3);

                // store
                _mm256_storeu_si256((__m256i *)&C[(i + 0) * bpl + j], vres0);
                _mm256_storeu_si256((__m256i *)&C[(i + 1) * bpl + j], vres1);
                _mm256_storeu_si256((__m256i *)&C[(i + 2) * bpl + j], vres2);
                _mm256_storeu_si256((__m256i *)&C[(i + 3) * bpl + j], vres3);
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
        for (int j = 0; j < bpt - (bpv - 1); j += bpv)
        {
            for (int k = 0; k < L1 - 1; k += 2)
            {
                char a00, a01, a10, a11, b00, b01, b10, b11, c00, c01, c10, c11, sum00, sum01, sum10, sum11;

                // load
                a00 = A[(i + 0) * bpl + (k + 0) / 8] & (1 << ((k + 0) % 8)) ? 0xff : 0x00;
                a01 = A[(i + 0) * bpl + (k + 1) / 8] & (1 << ((k + 1) % 8)) ? 0xff : 0x00;
                a10 = A[(i + 1) * bpl + (k + 0) / 8] & (1 << ((k + 0) % 8)) ? 0xff : 0x00;
                a11 = A[(i + 1) * bpl + (k + 1) / 8] & (1 << ((k + 1) % 8)) ? 0xff : 0x00;
                __m256i va00 = _mm256_set1_epi8(a00);
                __m256i va01 = _mm256_set1_epi8(a01);
                __m256i va10 = _mm256_set1_epi8(a10);
                __m256i va11 = _mm256_set1_epi8(a11);

                __m256i vb0 = _mm256_loadu_si256((__m256i const *)&B[(k + 0) * bpl + j]);
                __m256i vb1 = _mm256_loadu_si256((__m256i const *)&B[(k + 1) * bpl + j]);
                __m256i vc0 = _mm256_loadu_si256((__m256i const *)&C[(i + 0) * bpl + j]);
                __m256i vc1 = _mm256_loadu_si256((__m256i const *)&C[(i + 1) * bpl + j]);

                // for k + 0
                __m256i vsum0 = _mm256_and_si256(va00, vb0);
                __m256i vsum1 = _mm256_and_si256(va10, vb0);

                vc0 = _mm256_or_si256(vc0, vsum0);
                vc1 = _mm256_or_si256(vc1, vsum1);

                // for k + 1
                vsum0 = _mm256_and_si256(va01, vb1);
                vsum1 = _mm256_and_si256(va11, vb1);

                vc0 = _mm256_or_si256(vc0, vsum0);
                vc1 = _mm256_or_si256(vc1, vsum1);

                // store
                _mm256_storeu_si256((__m256i *)&C[(i + 0) * bpl + j], vc0);
                _mm256_storeu_si256((__m256i *)&C[(i + 1) * bpl + j], vc1);
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
    return FWT(C, C, C, N, 256);
}
