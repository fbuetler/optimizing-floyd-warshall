#include <immintrin.h>

#include "fw.h"

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
            double a_0_k = A[(i + 0) * N + k];
            double a_1_k = A[(i + 1) * N + k];
            double a_2_k = A[(i + 2) * N + k];
            double a_3_k = A[(i + 3) * N + k];
            double a_4_k = A[(i + 4) * N + k];
            double a_5_k = A[(i + 5) * N + k];
            double a_6_k = A[(i + 6) * N + k];
            double a_7_k = A[(i + 7) * N + k];
            double a_8_k = A[(i + 8) * N + k];
            double a_9_k = A[(i + 9) * N + k];
            double a_10_k = A[(i + 10) * N + k];
            double a_11_k = A[(i + 11) * N + k];
            double a_12_k = A[(i + 12) * N + k];
            double a_13_k = A[(i + 13) * N + k];
            double a_14_k = A[(i + 14) * N + k];
            double a_15_k = A[(i + 15) * N + k];
            for (int j = 0; j < L1 - 3; j += 4)
            {
                // load
                double c_0_0 = C[(i + 0) * N + j + 0];
                double c_0_1 = C[(i + 0) * N + j + 1];
                double c_0_2 = C[(i + 0) * N + j + 2];
                double c_0_3 = C[(i + 0) * N + j + 3];
                double c_1_0 = C[(i + 1) * N + j + 0];
                double c_1_1 = C[(i + 1) * N + j + 1];
                double c_1_2 = C[(i + 1) * N + j + 2];
                double c_1_3 = C[(i + 1) * N + j + 3];
                double c_2_0 = C[(i + 2) * N + j + 0];
                double c_2_1 = C[(i + 2) * N + j + 1];
                double c_2_2 = C[(i + 2) * N + j + 2];
                double c_2_3 = C[(i + 2) * N + j + 3];
                double c_3_0 = C[(i + 3) * N + j + 0];
                double c_3_1 = C[(i + 3) * N + j + 1];
                double c_3_2 = C[(i + 3) * N + j + 2];
                double c_3_3 = C[(i + 3) * N + j + 3];
                double c_4_0 = C[(i + 4) * N + j + 0];
                double c_4_1 = C[(i + 4) * N + j + 1];
                double c_4_2 = C[(i + 4) * N + j + 2];
                double c_4_3 = C[(i + 4) * N + j + 3];
                double c_5_0 = C[(i + 5) * N + j + 0];
                double c_5_1 = C[(i + 5) * N + j + 1];
                double c_5_2 = C[(i + 5) * N + j + 2];
                double c_5_3 = C[(i + 5) * N + j + 3];
                double c_6_0 = C[(i + 6) * N + j + 0];
                double c_6_1 = C[(i + 6) * N + j + 1];
                double c_6_2 = C[(i + 6) * N + j + 2];
                double c_6_3 = C[(i + 6) * N + j + 3];
                double c_7_0 = C[(i + 7) * N + j + 0];
                double c_7_1 = C[(i + 7) * N + j + 1];
                double c_7_2 = C[(i + 7) * N + j + 2];
                double c_7_3 = C[(i + 7) * N + j + 3];
                double c_8_0 = C[(i + 8) * N + j + 0];
                double c_8_1 = C[(i + 8) * N + j + 1];
                double c_8_2 = C[(i + 8) * N + j + 2];
                double c_8_3 = C[(i + 8) * N + j + 3];
                double c_9_0 = C[(i + 9) * N + j + 0];
                double c_9_1 = C[(i + 9) * N + j + 1];
                double c_9_2 = C[(i + 9) * N + j + 2];
                double c_9_3 = C[(i + 9) * N + j + 3];
                double c_10_0 = C[(i + 10) * N + j + 0];
                double c_10_1 = C[(i + 10) * N + j + 1];
                double c_10_2 = C[(i + 10) * N + j + 2];
                double c_10_3 = C[(i + 10) * N + j + 3];
                double c_11_0 = C[(i + 11) * N + j + 0];
                double c_11_1 = C[(i + 11) * N + j + 1];
                double c_11_2 = C[(i + 11) * N + j + 2];
                double c_11_3 = C[(i + 11) * N + j + 3];
                double c_12_0 = C[(i + 12) * N + j + 0];
                double c_12_1 = C[(i + 12) * N + j + 1];
                double c_12_2 = C[(i + 12) * N + j + 2];
                double c_12_3 = C[(i + 12) * N + j + 3];
                double c_13_0 = C[(i + 13) * N + j + 0];
                double c_13_1 = C[(i + 13) * N + j + 1];
                double c_13_2 = C[(i + 13) * N + j + 2];
                double c_13_3 = C[(i + 13) * N + j + 3];
                double c_14_0 = C[(i + 14) * N + j + 0];
                double c_14_1 = C[(i + 14) * N + j + 1];
                double c_14_2 = C[(i + 14) * N + j + 2];
                double c_14_3 = C[(i + 14) * N + j + 3];
                double c_15_0 = C[(i + 15) * N + j + 0];
                double c_15_1 = C[(i + 15) * N + j + 1];
                double c_15_2 = C[(i + 15) * N + j + 2];
                double c_15_3 = C[(i + 15) * N + j + 3];
                

                double b_k_0 = B[k * N + j + 0];
                double b_k_1 = B[k * N + j + 1];
                double b_k_2 = B[k * N + j + 2];
                double b_k_3 = B[k * N + j + 3];
                

                // op 1
                double res_0_0 = ADD(a_0_k, b_k_0);
                double res_0_1 = ADD(a_0_k, b_k_1);
                double res_0_2 = ADD(a_0_k, b_k_2);
                double res_0_3 = ADD(a_0_k, b_k_3);
                double res_1_0 = ADD(a_1_k, b_k_0);
                double res_1_1 = ADD(a_1_k, b_k_1);
                double res_1_2 = ADD(a_1_k, b_k_2);
                double res_1_3 = ADD(a_1_k, b_k_3);
                double res_2_0 = ADD(a_2_k, b_k_0);
                double res_2_1 = ADD(a_2_k, b_k_1);
                double res_2_2 = ADD(a_2_k, b_k_2);
                double res_2_3 = ADD(a_2_k, b_k_3);
                double res_3_0 = ADD(a_3_k, b_k_0);
                double res_3_1 = ADD(a_3_k, b_k_1);
                double res_3_2 = ADD(a_3_k, b_k_2);
                double res_3_3 = ADD(a_3_k, b_k_3);
                double res_4_0 = ADD(a_4_k, b_k_0);
                double res_4_1 = ADD(a_4_k, b_k_1);
                double res_4_2 = ADD(a_4_k, b_k_2);
                double res_4_3 = ADD(a_4_k, b_k_3);
                double res_5_0 = ADD(a_5_k, b_k_0);
                double res_5_1 = ADD(a_5_k, b_k_1);
                double res_5_2 = ADD(a_5_k, b_k_2);
                double res_5_3 = ADD(a_5_k, b_k_3);
                double res_6_0 = ADD(a_6_k, b_k_0);
                double res_6_1 = ADD(a_6_k, b_k_1);
                double res_6_2 = ADD(a_6_k, b_k_2);
                double res_6_3 = ADD(a_6_k, b_k_3);
                double res_7_0 = ADD(a_7_k, b_k_0);
                double res_7_1 = ADD(a_7_k, b_k_1);
                double res_7_2 = ADD(a_7_k, b_k_2);
                double res_7_3 = ADD(a_7_k, b_k_3);
                double res_8_0 = ADD(a_8_k, b_k_0);
                double res_8_1 = ADD(a_8_k, b_k_1);
                double res_8_2 = ADD(a_8_k, b_k_2);
                double res_8_3 = ADD(a_8_k, b_k_3);
                double res_9_0 = ADD(a_9_k, b_k_0);
                double res_9_1 = ADD(a_9_k, b_k_1);
                double res_9_2 = ADD(a_9_k, b_k_2);
                double res_9_3 = ADD(a_9_k, b_k_3);
                double res_10_0 = ADD(a_10_k, b_k_0);
                double res_10_1 = ADD(a_10_k, b_k_1);
                double res_10_2 = ADD(a_10_k, b_k_2);
                double res_10_3 = ADD(a_10_k, b_k_3);
                double res_11_0 = ADD(a_11_k, b_k_0);
                double res_11_1 = ADD(a_11_k, b_k_1);
                double res_11_2 = ADD(a_11_k, b_k_2);
                double res_11_3 = ADD(a_11_k, b_k_3);
                double res_12_0 = ADD(a_12_k, b_k_0);
                double res_12_1 = ADD(a_12_k, b_k_1);
                double res_12_2 = ADD(a_12_k, b_k_2);
                double res_12_3 = ADD(a_12_k, b_k_3);
                double res_13_0 = ADD(a_13_k, b_k_0);
                double res_13_1 = ADD(a_13_k, b_k_1);
                double res_13_2 = ADD(a_13_k, b_k_2);
                double res_13_3 = ADD(a_13_k, b_k_3);
                double res_14_0 = ADD(a_14_k, b_k_0);
                double res_14_1 = ADD(a_14_k, b_k_1);
                double res_14_2 = ADD(a_14_k, b_k_2);
                double res_14_3 = ADD(a_14_k, b_k_3);
                double res_15_0 = ADD(a_15_k, b_k_0);
                double res_15_1 = ADD(a_15_k, b_k_1);
                double res_15_2 = ADD(a_15_k, b_k_2);
                double res_15_3 = ADD(a_15_k, b_k_3);
                

                // op 2
                c_0_0 = MIN(c_0_0, res_0_0);
                c_0_1 = MIN(c_0_1, res_0_1);
                c_0_2 = MIN(c_0_2, res_0_2);
                c_0_3 = MIN(c_0_3, res_0_3);
                c_1_0 = MIN(c_1_0, res_1_0);
                c_1_1 = MIN(c_1_1, res_1_1);
                c_1_2 = MIN(c_1_2, res_1_2);
                c_1_3 = MIN(c_1_3, res_1_3);
                c_2_0 = MIN(c_2_0, res_2_0);
                c_2_1 = MIN(c_2_1, res_2_1);
                c_2_2 = MIN(c_2_2, res_2_2);
                c_2_3 = MIN(c_2_3, res_2_3);
                c_3_0 = MIN(c_3_0, res_3_0);
                c_3_1 = MIN(c_3_1, res_3_1);
                c_3_2 = MIN(c_3_2, res_3_2);
                c_3_3 = MIN(c_3_3, res_3_3);
                c_4_0 = MIN(c_4_0, res_4_0);
                c_4_1 = MIN(c_4_1, res_4_1);
                c_4_2 = MIN(c_4_2, res_4_2);
                c_4_3 = MIN(c_4_3, res_4_3);
                c_5_0 = MIN(c_5_0, res_5_0);
                c_5_1 = MIN(c_5_1, res_5_1);
                c_5_2 = MIN(c_5_2, res_5_2);
                c_5_3 = MIN(c_5_3, res_5_3);
                c_6_0 = MIN(c_6_0, res_6_0);
                c_6_1 = MIN(c_6_1, res_6_1);
                c_6_2 = MIN(c_6_2, res_6_2);
                c_6_3 = MIN(c_6_3, res_6_3);
                c_7_0 = MIN(c_7_0, res_7_0);
                c_7_1 = MIN(c_7_1, res_7_1);
                c_7_2 = MIN(c_7_2, res_7_2);
                c_7_3 = MIN(c_7_3, res_7_3);
                c_8_0 = MIN(c_8_0, res_8_0);
                c_8_1 = MIN(c_8_1, res_8_1);
                c_8_2 = MIN(c_8_2, res_8_2);
                c_8_3 = MIN(c_8_3, res_8_3);
                c_9_0 = MIN(c_9_0, res_9_0);
                c_9_1 = MIN(c_9_1, res_9_1);
                c_9_2 = MIN(c_9_2, res_9_2);
                c_9_3 = MIN(c_9_3, res_9_3);
                c_10_0 = MIN(c_10_0, res_10_0);
                c_10_1 = MIN(c_10_1, res_10_1);
                c_10_2 = MIN(c_10_2, res_10_2);
                c_10_3 = MIN(c_10_3, res_10_3);
                c_11_0 = MIN(c_11_0, res_11_0);
                c_11_1 = MIN(c_11_1, res_11_1);
                c_11_2 = MIN(c_11_2, res_11_2);
                c_11_3 = MIN(c_11_3, res_11_3);
                c_12_0 = MIN(c_12_0, res_12_0);
                c_12_1 = MIN(c_12_1, res_12_1);
                c_12_2 = MIN(c_12_2, res_12_2);
                c_12_3 = MIN(c_12_3, res_12_3);
                c_13_0 = MIN(c_13_0, res_13_0);
                c_13_1 = MIN(c_13_1, res_13_1);
                c_13_2 = MIN(c_13_2, res_13_2);
                c_13_3 = MIN(c_13_3, res_13_3);
                c_14_0 = MIN(c_14_0, res_14_0);
                c_14_1 = MIN(c_14_1, res_14_1);
                c_14_2 = MIN(c_14_2, res_14_2);
                c_14_3 = MIN(c_14_3, res_14_3);
                c_15_0 = MIN(c_15_0, res_15_0);
                c_15_1 = MIN(c_15_1, res_15_1);
                c_15_2 = MIN(c_15_2, res_15_2);
                c_15_3 = MIN(c_15_3, res_15_3);
                

                // store
                C[(i + 0) * N + j + 0] = c_0_0;
                C[(i + 0) * N + j + 1] = c_0_1;
                C[(i + 0) * N + j + 2] = c_0_2;
                C[(i + 0) * N + j + 3] = c_0_3;
                C[(i + 1) * N + j + 0] = c_1_0;
                C[(i + 1) * N + j + 1] = c_1_1;
                C[(i + 1) * N + j + 2] = c_1_2;
                C[(i + 1) * N + j + 3] = c_1_3;
                C[(i + 2) * N + j + 0] = c_2_0;
                C[(i + 2) * N + j + 1] = c_2_1;
                C[(i + 2) * N + j + 2] = c_2_2;
                C[(i + 2) * N + j + 3] = c_2_3;
                C[(i + 3) * N + j + 0] = c_3_0;
                C[(i + 3) * N + j + 1] = c_3_1;
                C[(i + 3) * N + j + 2] = c_3_2;
                C[(i + 3) * N + j + 3] = c_3_3;
                C[(i + 4) * N + j + 0] = c_4_0;
                C[(i + 4) * N + j + 1] = c_4_1;
                C[(i + 4) * N + j + 2] = c_4_2;
                C[(i + 4) * N + j + 3] = c_4_3;
                C[(i + 5) * N + j + 0] = c_5_0;
                C[(i + 5) * N + j + 1] = c_5_1;
                C[(i + 5) * N + j + 2] = c_5_2;
                C[(i + 5) * N + j + 3] = c_5_3;
                C[(i + 6) * N + j + 0] = c_6_0;
                C[(i + 6) * N + j + 1] = c_6_1;
                C[(i + 6) * N + j + 2] = c_6_2;
                C[(i + 6) * N + j + 3] = c_6_3;
                C[(i + 7) * N + j + 0] = c_7_0;
                C[(i + 7) * N + j + 1] = c_7_1;
                C[(i + 7) * N + j + 2] = c_7_2;
                C[(i + 7) * N + j + 3] = c_7_3;
                C[(i + 8) * N + j + 0] = c_8_0;
                C[(i + 8) * N + j + 1] = c_8_1;
                C[(i + 8) * N + j + 2] = c_8_2;
                C[(i + 8) * N + j + 3] = c_8_3;
                C[(i + 9) * N + j + 0] = c_9_0;
                C[(i + 9) * N + j + 1] = c_9_1;
                C[(i + 9) * N + j + 2] = c_9_2;
                C[(i + 9) * N + j + 3] = c_9_3;
                C[(i + 10) * N + j + 0] = c_10_0;
                C[(i + 10) * N + j + 1] = c_10_1;
                C[(i + 10) * N + j + 2] = c_10_2;
                C[(i + 10) * N + j + 3] = c_10_3;
                C[(i + 11) * N + j + 0] = c_11_0;
                C[(i + 11) * N + j + 1] = c_11_1;
                C[(i + 11) * N + j + 2] = c_11_2;
                C[(i + 11) * N + j + 3] = c_11_3;
                C[(i + 12) * N + j + 0] = c_12_0;
                C[(i + 12) * N + j + 1] = c_12_1;
                C[(i + 12) * N + j + 2] = c_12_2;
                C[(i + 12) * N + j + 3] = c_12_3;
                C[(i + 13) * N + j + 0] = c_13_0;
                C[(i + 13) * N + j + 1] = c_13_1;
                C[(i + 13) * N + j + 2] = c_13_2;
                C[(i + 13) * N + j + 3] = c_13_3;
                C[(i + 14) * N + j + 0] = c_14_0;
                C[(i + 14) * N + j + 1] = c_14_1;
                C[(i + 14) * N + j + 2] = c_14_2;
                C[(i + 14) * N + j + 3] = c_14_3;
                C[(i + 15) * N + j + 0] = c_15_0;
                C[(i + 15) * N + j + 1] = c_15_1;
                C[(i + 15) * N + j + 2] = c_15_2;
                C[(i + 15) * N + j + 3] = c_15_3;
                
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
                double a_0_0 = A[(i + 0) * N + (k + 0)];
                

                double b_0_0 = B[(k + 0) * N + (j + 0)];
                double b_0_1 = B[(k + 0) * N + (j + 1)];
                double b_0_2 = B[(k + 0) * N + (j + 2)];
                double b_0_3 = B[(k + 0) * N + (j + 3)];
                

                double c_0_0 = C[(i + 0) * N + j + 0];
                double c_0_1 = C[(i + 0) * N + j + 1];
                double c_0_2 = C[(i + 0) * N + j + 2];
                double c_0_3 = C[(i + 0) * N + j + 3];
                

                // for k + 0
                double res_0_0 = ADD(a_0_0, b_0_0);
                double res_0_1 = ADD(a_0_0, b_0_1);
                double res_0_2 = ADD(a_0_0, b_0_2);
                double res_0_3 = ADD(a_0_0, b_0_3);
                

                c_0_0 = MIN(c_0_0, res_0_0);
                c_0_1 = MIN(c_0_1, res_0_1);
                c_0_2 = MIN(c_0_2, res_0_2);
                c_0_3 = MIN(c_0_3, res_0_3);
                
                // store
                C[(i + 0) * N + j + 0] = c_0_0;
                C[(i + 0) * N + j + 1] = c_0_1;
                C[(i + 0) * N + j + 2] = c_0_2;
                C[(i + 0) * N + j + 3] = c_0_3;
                
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