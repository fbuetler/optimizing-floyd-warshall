#include <immintrin.h>

#include "fw.h"

// iterative FW algorithm (FWI)
// tiling factors Ui and Uj are set to 16 and 4, respectively
int FWI(double *A, double *B, double *C, int N)
{
    // NOTE 1: We assume Ui and Uj divide N for simplicity
    // NOTE 2: If generating vector code, we require that Uj is divisible by 4
    for (int k = 0; k < N; k++)
    {
        for (int i = 0; i < N - 15; i += 16)
        {
            double a_i0_k = A[(i + 0) * N + k];
            double a_i1_k = A[(i + 1) * N + k];
            double a_i2_k = A[(i + 2) * N + k];
            double a_i3_k = A[(i + 3) * N + k];
            double a_i4_k = A[(i + 4) * N + k];
            double a_i5_k = A[(i + 5) * N + k];
            double a_i6_k = A[(i + 6) * N + k];
            double a_i7_k = A[(i + 7) * N + k];
            double a_i8_k = A[(i + 8) * N + k];
            double a_i9_k = A[(i + 9) * N + k];
            double a_i10_k = A[(i + 10) * N + k];
            double a_i11_k = A[(i + 11) * N + k];
            double a_i12_k = A[(i + 12) * N + k];
            double a_i13_k = A[(i + 13) * N + k];
            double a_i14_k = A[(i + 14) * N + k];
            double a_i15_k = A[(i + 15) * N + k];
            for (int j = 0; j < N - 3; j += 4)
            {
                // load
                double c_i0_j0 = C[(i + 0) * N + j + 0];
                double c_i0_j1 = C[(i + 0) * N + j + 1];
                double c_i0_j2 = C[(i + 0) * N + j + 2];
                double c_i0_j3 = C[(i + 0) * N + j + 3];
                double c_i1_j0 = C[(i + 1) * N + j + 0];
                double c_i1_j1 = C[(i + 1) * N + j + 1];
                double c_i1_j2 = C[(i + 1) * N + j + 2];
                double c_i1_j3 = C[(i + 1) * N + j + 3];
                double c_i2_j0 = C[(i + 2) * N + j + 0];
                double c_i2_j1 = C[(i + 2) * N + j + 1];
                double c_i2_j2 = C[(i + 2) * N + j + 2];
                double c_i2_j3 = C[(i + 2) * N + j + 3];
                double c_i3_j0 = C[(i + 3) * N + j + 0];
                double c_i3_j1 = C[(i + 3) * N + j + 1];
                double c_i3_j2 = C[(i + 3) * N + j + 2];
                double c_i3_j3 = C[(i + 3) * N + j + 3];
                double c_i4_j0 = C[(i + 4) * N + j + 0];
                double c_i4_j1 = C[(i + 4) * N + j + 1];
                double c_i4_j2 = C[(i + 4) * N + j + 2];
                double c_i4_j3 = C[(i + 4) * N + j + 3];
                double c_i5_j0 = C[(i + 5) * N + j + 0];
                double c_i5_j1 = C[(i + 5) * N + j + 1];
                double c_i5_j2 = C[(i + 5) * N + j + 2];
                double c_i5_j3 = C[(i + 5) * N + j + 3];
                double c_i6_j0 = C[(i + 6) * N + j + 0];
                double c_i6_j1 = C[(i + 6) * N + j + 1];
                double c_i6_j2 = C[(i + 6) * N + j + 2];
                double c_i6_j3 = C[(i + 6) * N + j + 3];
                double c_i7_j0 = C[(i + 7) * N + j + 0];
                double c_i7_j1 = C[(i + 7) * N + j + 1];
                double c_i7_j2 = C[(i + 7) * N + j + 2];
                double c_i7_j3 = C[(i + 7) * N + j + 3];
                double c_i8_j0 = C[(i + 8) * N + j + 0];
                double c_i8_j1 = C[(i + 8) * N + j + 1];
                double c_i8_j2 = C[(i + 8) * N + j + 2];
                double c_i8_j3 = C[(i + 8) * N + j + 3];
                double c_i9_j0 = C[(i + 9) * N + j + 0];
                double c_i9_j1 = C[(i + 9) * N + j + 1];
                double c_i9_j2 = C[(i + 9) * N + j + 2];
                double c_i9_j3 = C[(i + 9) * N + j + 3];
                double c_i10_j0 = C[(i + 10) * N + j + 0];
                double c_i10_j1 = C[(i + 10) * N + j + 1];
                double c_i10_j2 = C[(i + 10) * N + j + 2];
                double c_i10_j3 = C[(i + 10) * N + j + 3];
                double c_i11_j0 = C[(i + 11) * N + j + 0];
                double c_i11_j1 = C[(i + 11) * N + j + 1];
                double c_i11_j2 = C[(i + 11) * N + j + 2];
                double c_i11_j3 = C[(i + 11) * N + j + 3];
                double c_i12_j0 = C[(i + 12) * N + j + 0];
                double c_i12_j1 = C[(i + 12) * N + j + 1];
                double c_i12_j2 = C[(i + 12) * N + j + 2];
                double c_i12_j3 = C[(i + 12) * N + j + 3];
                double c_i13_j0 = C[(i + 13) * N + j + 0];
                double c_i13_j1 = C[(i + 13) * N + j + 1];
                double c_i13_j2 = C[(i + 13) * N + j + 2];
                double c_i13_j3 = C[(i + 13) * N + j + 3];
                double c_i14_j0 = C[(i + 14) * N + j + 0];
                double c_i14_j1 = C[(i + 14) * N + j + 1];
                double c_i14_j2 = C[(i + 14) * N + j + 2];
                double c_i14_j3 = C[(i + 14) * N + j + 3];
                double c_i15_j0 = C[(i + 15) * N + j + 0];
                double c_i15_j1 = C[(i + 15) * N + j + 1];
                double c_i15_j2 = C[(i + 15) * N + j + 2];
                double c_i15_j3 = C[(i + 15) * N + j + 3];
                

                double b_k_j0 = B[k * N + j + 0];
                double b_k_j1 = B[k * N + j + 1];
                double b_k_j2 = B[k * N + j + 2];
                double b_k_j3 = B[k * N + j + 3];
                

                // op 1
                double res_i0_j0 = ADD(a_i0_k, b_k_j0);
                double res_i0_j1 = ADD(a_i0_k, b_k_j1);
                double res_i0_j2 = ADD(a_i0_k, b_k_j2);
                double res_i0_j3 = ADD(a_i0_k, b_k_j3);
                double res_i1_j0 = ADD(a_i1_k, b_k_j0);
                double res_i1_j1 = ADD(a_i1_k, b_k_j1);
                double res_i1_j2 = ADD(a_i1_k, b_k_j2);
                double res_i1_j3 = ADD(a_i1_k, b_k_j3);
                double res_i2_j0 = ADD(a_i2_k, b_k_j0);
                double res_i2_j1 = ADD(a_i2_k, b_k_j1);
                double res_i2_j2 = ADD(a_i2_k, b_k_j2);
                double res_i2_j3 = ADD(a_i2_k, b_k_j3);
                double res_i3_j0 = ADD(a_i3_k, b_k_j0);
                double res_i3_j1 = ADD(a_i3_k, b_k_j1);
                double res_i3_j2 = ADD(a_i3_k, b_k_j2);
                double res_i3_j3 = ADD(a_i3_k, b_k_j3);
                double res_i4_j0 = ADD(a_i4_k, b_k_j0);
                double res_i4_j1 = ADD(a_i4_k, b_k_j1);
                double res_i4_j2 = ADD(a_i4_k, b_k_j2);
                double res_i4_j3 = ADD(a_i4_k, b_k_j3);
                double res_i5_j0 = ADD(a_i5_k, b_k_j0);
                double res_i5_j1 = ADD(a_i5_k, b_k_j1);
                double res_i5_j2 = ADD(a_i5_k, b_k_j2);
                double res_i5_j3 = ADD(a_i5_k, b_k_j3);
                double res_i6_j0 = ADD(a_i6_k, b_k_j0);
                double res_i6_j1 = ADD(a_i6_k, b_k_j1);
                double res_i6_j2 = ADD(a_i6_k, b_k_j2);
                double res_i6_j3 = ADD(a_i6_k, b_k_j3);
                double res_i7_j0 = ADD(a_i7_k, b_k_j0);
                double res_i7_j1 = ADD(a_i7_k, b_k_j1);
                double res_i7_j2 = ADD(a_i7_k, b_k_j2);
                double res_i7_j3 = ADD(a_i7_k, b_k_j3);
                double res_i8_j0 = ADD(a_i8_k, b_k_j0);
                double res_i8_j1 = ADD(a_i8_k, b_k_j1);
                double res_i8_j2 = ADD(a_i8_k, b_k_j2);
                double res_i8_j3 = ADD(a_i8_k, b_k_j3);
                double res_i9_j0 = ADD(a_i9_k, b_k_j0);
                double res_i9_j1 = ADD(a_i9_k, b_k_j1);
                double res_i9_j2 = ADD(a_i9_k, b_k_j2);
                double res_i9_j3 = ADD(a_i9_k, b_k_j3);
                double res_i10_j0 = ADD(a_i10_k, b_k_j0);
                double res_i10_j1 = ADD(a_i10_k, b_k_j1);
                double res_i10_j2 = ADD(a_i10_k, b_k_j2);
                double res_i10_j3 = ADD(a_i10_k, b_k_j3);
                double res_i11_j0 = ADD(a_i11_k, b_k_j0);
                double res_i11_j1 = ADD(a_i11_k, b_k_j1);
                double res_i11_j2 = ADD(a_i11_k, b_k_j2);
                double res_i11_j3 = ADD(a_i11_k, b_k_j3);
                double res_i12_j0 = ADD(a_i12_k, b_k_j0);
                double res_i12_j1 = ADD(a_i12_k, b_k_j1);
                double res_i12_j2 = ADD(a_i12_k, b_k_j2);
                double res_i12_j3 = ADD(a_i12_k, b_k_j3);
                double res_i13_j0 = ADD(a_i13_k, b_k_j0);
                double res_i13_j1 = ADD(a_i13_k, b_k_j1);
                double res_i13_j2 = ADD(a_i13_k, b_k_j2);
                double res_i13_j3 = ADD(a_i13_k, b_k_j3);
                double res_i14_j0 = ADD(a_i14_k, b_k_j0);
                double res_i14_j1 = ADD(a_i14_k, b_k_j1);
                double res_i14_j2 = ADD(a_i14_k, b_k_j2);
                double res_i14_j3 = ADD(a_i14_k, b_k_j3);
                double res_i15_j0 = ADD(a_i15_k, b_k_j0);
                double res_i15_j1 = ADD(a_i15_k, b_k_j1);
                double res_i15_j2 = ADD(a_i15_k, b_k_j2);
                double res_i15_j3 = ADD(a_i15_k, b_k_j3);
                

                // op 2
                c_i0_j0 = MIN(c_i0_j0, res_i0_j0);
                c_i0_j1 = MIN(c_i0_j1, res_i0_j1);
                c_i0_j2 = MIN(c_i0_j2, res_i0_j2);
                c_i0_j3 = MIN(c_i0_j3, res_i0_j3);
                c_i1_j0 = MIN(c_i1_j0, res_i1_j0);
                c_i1_j1 = MIN(c_i1_j1, res_i1_j1);
                c_i1_j2 = MIN(c_i1_j2, res_i1_j2);
                c_i1_j3 = MIN(c_i1_j3, res_i1_j3);
                c_i2_j0 = MIN(c_i2_j0, res_i2_j0);
                c_i2_j1 = MIN(c_i2_j1, res_i2_j1);
                c_i2_j2 = MIN(c_i2_j2, res_i2_j2);
                c_i2_j3 = MIN(c_i2_j3, res_i2_j3);
                c_i3_j0 = MIN(c_i3_j0, res_i3_j0);
                c_i3_j1 = MIN(c_i3_j1, res_i3_j1);
                c_i3_j2 = MIN(c_i3_j2, res_i3_j2);
                c_i3_j3 = MIN(c_i3_j3, res_i3_j3);
                c_i4_j0 = MIN(c_i4_j0, res_i4_j0);
                c_i4_j1 = MIN(c_i4_j1, res_i4_j1);
                c_i4_j2 = MIN(c_i4_j2, res_i4_j2);
                c_i4_j3 = MIN(c_i4_j3, res_i4_j3);
                c_i5_j0 = MIN(c_i5_j0, res_i5_j0);
                c_i5_j1 = MIN(c_i5_j1, res_i5_j1);
                c_i5_j2 = MIN(c_i5_j2, res_i5_j2);
                c_i5_j3 = MIN(c_i5_j3, res_i5_j3);
                c_i6_j0 = MIN(c_i6_j0, res_i6_j0);
                c_i6_j1 = MIN(c_i6_j1, res_i6_j1);
                c_i6_j2 = MIN(c_i6_j2, res_i6_j2);
                c_i6_j3 = MIN(c_i6_j3, res_i6_j3);
                c_i7_j0 = MIN(c_i7_j0, res_i7_j0);
                c_i7_j1 = MIN(c_i7_j1, res_i7_j1);
                c_i7_j2 = MIN(c_i7_j2, res_i7_j2);
                c_i7_j3 = MIN(c_i7_j3, res_i7_j3);
                c_i8_j0 = MIN(c_i8_j0, res_i8_j0);
                c_i8_j1 = MIN(c_i8_j1, res_i8_j1);
                c_i8_j2 = MIN(c_i8_j2, res_i8_j2);
                c_i8_j3 = MIN(c_i8_j3, res_i8_j3);
                c_i9_j0 = MIN(c_i9_j0, res_i9_j0);
                c_i9_j1 = MIN(c_i9_j1, res_i9_j1);
                c_i9_j2 = MIN(c_i9_j2, res_i9_j2);
                c_i9_j3 = MIN(c_i9_j3, res_i9_j3);
                c_i10_j0 = MIN(c_i10_j0, res_i10_j0);
                c_i10_j1 = MIN(c_i10_j1, res_i10_j1);
                c_i10_j2 = MIN(c_i10_j2, res_i10_j2);
                c_i10_j3 = MIN(c_i10_j3, res_i10_j3);
                c_i11_j0 = MIN(c_i11_j0, res_i11_j0);
                c_i11_j1 = MIN(c_i11_j1, res_i11_j1);
                c_i11_j2 = MIN(c_i11_j2, res_i11_j2);
                c_i11_j3 = MIN(c_i11_j3, res_i11_j3);
                c_i12_j0 = MIN(c_i12_j0, res_i12_j0);
                c_i12_j1 = MIN(c_i12_j1, res_i12_j1);
                c_i12_j2 = MIN(c_i12_j2, res_i12_j2);
                c_i12_j3 = MIN(c_i12_j3, res_i12_j3);
                c_i13_j0 = MIN(c_i13_j0, res_i13_j0);
                c_i13_j1 = MIN(c_i13_j1, res_i13_j1);
                c_i13_j2 = MIN(c_i13_j2, res_i13_j2);
                c_i13_j3 = MIN(c_i13_j3, res_i13_j3);
                c_i14_j0 = MIN(c_i14_j0, res_i14_j0);
                c_i14_j1 = MIN(c_i14_j1, res_i14_j1);
                c_i14_j2 = MIN(c_i14_j2, res_i14_j2);
                c_i14_j3 = MIN(c_i14_j3, res_i14_j3);
                c_i15_j0 = MIN(c_i15_j0, res_i15_j0);
                c_i15_j1 = MIN(c_i15_j1, res_i15_j1);
                c_i15_j2 = MIN(c_i15_j2, res_i15_j2);
                c_i15_j3 = MIN(c_i15_j3, res_i15_j3);
                

                // store
                C[(i + 0) * N + j + 0] = c_i0_j0;
                C[(i + 0) * N + j + 1] = c_i0_j1;
                C[(i + 0) * N + j + 2] = c_i0_j2;
                C[(i + 0) * N + j + 3] = c_i0_j3;
                C[(i + 1) * N + j + 0] = c_i1_j0;
                C[(i + 1) * N + j + 1] = c_i1_j1;
                C[(i + 1) * N + j + 2] = c_i1_j2;
                C[(i + 1) * N + j + 3] = c_i1_j3;
                C[(i + 2) * N + j + 0] = c_i2_j0;
                C[(i + 2) * N + j + 1] = c_i2_j1;
                C[(i + 2) * N + j + 2] = c_i2_j2;
                C[(i + 2) * N + j + 3] = c_i2_j3;
                C[(i + 3) * N + j + 0] = c_i3_j0;
                C[(i + 3) * N + j + 1] = c_i3_j1;
                C[(i + 3) * N + j + 2] = c_i3_j2;
                C[(i + 3) * N + j + 3] = c_i3_j3;
                C[(i + 4) * N + j + 0] = c_i4_j0;
                C[(i + 4) * N + j + 1] = c_i4_j1;
                C[(i + 4) * N + j + 2] = c_i4_j2;
                C[(i + 4) * N + j + 3] = c_i4_j3;
                C[(i + 5) * N + j + 0] = c_i5_j0;
                C[(i + 5) * N + j + 1] = c_i5_j1;
                C[(i + 5) * N + j + 2] = c_i5_j2;
                C[(i + 5) * N + j + 3] = c_i5_j3;
                C[(i + 6) * N + j + 0] = c_i6_j0;
                C[(i + 6) * N + j + 1] = c_i6_j1;
                C[(i + 6) * N + j + 2] = c_i6_j2;
                C[(i + 6) * N + j + 3] = c_i6_j3;
                C[(i + 7) * N + j + 0] = c_i7_j0;
                C[(i + 7) * N + j + 1] = c_i7_j1;
                C[(i + 7) * N + j + 2] = c_i7_j2;
                C[(i + 7) * N + j + 3] = c_i7_j3;
                C[(i + 8) * N + j + 0] = c_i8_j0;
                C[(i + 8) * N + j + 1] = c_i8_j1;
                C[(i + 8) * N + j + 2] = c_i8_j2;
                C[(i + 8) * N + j + 3] = c_i8_j3;
                C[(i + 9) * N + j + 0] = c_i9_j0;
                C[(i + 9) * N + j + 1] = c_i9_j1;
                C[(i + 9) * N + j + 2] = c_i9_j2;
                C[(i + 9) * N + j + 3] = c_i9_j3;
                C[(i + 10) * N + j + 0] = c_i10_j0;
                C[(i + 10) * N + j + 1] = c_i10_j1;
                C[(i + 10) * N + j + 2] = c_i10_j2;
                C[(i + 10) * N + j + 3] = c_i10_j3;
                C[(i + 11) * N + j + 0] = c_i11_j0;
                C[(i + 11) * N + j + 1] = c_i11_j1;
                C[(i + 11) * N + j + 2] = c_i11_j2;
                C[(i + 11) * N + j + 3] = c_i11_j3;
                C[(i + 12) * N + j + 0] = c_i12_j0;
                C[(i + 12) * N + j + 1] = c_i12_j1;
                C[(i + 12) * N + j + 2] = c_i12_j2;
                C[(i + 12) * N + j + 3] = c_i12_j3;
                C[(i + 13) * N + j + 0] = c_i13_j0;
                C[(i + 13) * N + j + 1] = c_i13_j1;
                C[(i + 13) * N + j + 2] = c_i13_j2;
                C[(i + 13) * N + j + 3] = c_i13_j3;
                C[(i + 14) * N + j + 0] = c_i14_j0;
                C[(i + 14) * N + j + 1] = c_i14_j1;
                C[(i + 14) * N + j + 2] = c_i14_j2;
                C[(i + 14) * N + j + 3] = c_i14_j3;
                C[(i + 15) * N + j + 0] = c_i15_j0;
                C[(i + 15) * N + j + 1] = c_i15_j1;
                C[(i + 15) * N + j + 2] = c_i15_j2;
                C[(i + 15) * N + j + 3] = c_i15_j3;
                
            }
        }
    }
    return 0;
}

int floydWarshall(double *C, int N)
{
    return FWI(C, C, C, N);
}