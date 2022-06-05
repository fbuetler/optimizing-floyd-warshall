#include <immintrin.h>

#include "sp.h"

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
            __m256d a_i0_k = _mm256_broadcast_sd(&A[(i + 0) * N + k]);
            __m256d a_i1_k = _mm256_broadcast_sd(&A[(i + 1) * N + k]);
            __m256d a_i2_k = _mm256_broadcast_sd(&A[(i + 2) * N + k]);
            __m256d a_i3_k = _mm256_broadcast_sd(&A[(i + 3) * N + k]);
            __m256d a_i4_k = _mm256_broadcast_sd(&A[(i + 4) * N + k]);
            __m256d a_i5_k = _mm256_broadcast_sd(&A[(i + 5) * N + k]);
            __m256d a_i6_k = _mm256_broadcast_sd(&A[(i + 6) * N + k]);
            __m256d a_i7_k = _mm256_broadcast_sd(&A[(i + 7) * N + k]);
            __m256d a_i8_k = _mm256_broadcast_sd(&A[(i + 8) * N + k]);
            __m256d a_i9_k = _mm256_broadcast_sd(&A[(i + 9) * N + k]);
            __m256d a_i10_k = _mm256_broadcast_sd(&A[(i + 10) * N + k]);
            __m256d a_i11_k = _mm256_broadcast_sd(&A[(i + 11) * N + k]);
            __m256d a_i12_k = _mm256_broadcast_sd(&A[(i + 12) * N + k]);
            __m256d a_i13_k = _mm256_broadcast_sd(&A[(i + 13) * N + k]);
            __m256d a_i14_k = _mm256_broadcast_sd(&A[(i + 14) * N + k]);
            __m256d a_i15_k = _mm256_broadcast_sd(&A[(i + 15) * N + k]);
            for (int j = 0; j < N - 3; j += 4)
            {
                // load
                __m256d c_i0_j0 = _mm256_load_pd(&C[(i + 0) * N + j + 0]);
                __m256d c_i1_j0 = _mm256_load_pd(&C[(i + 1) * N + j + 0]);
                __m256d c_i2_j0 = _mm256_load_pd(&C[(i + 2) * N + j + 0]);
                __m256d c_i3_j0 = _mm256_load_pd(&C[(i + 3) * N + j + 0]);
                __m256d c_i4_j0 = _mm256_load_pd(&C[(i + 4) * N + j + 0]);
                __m256d c_i5_j0 = _mm256_load_pd(&C[(i + 5) * N + j + 0]);
                __m256d c_i6_j0 = _mm256_load_pd(&C[(i + 6) * N + j + 0]);
                __m256d c_i7_j0 = _mm256_load_pd(&C[(i + 7) * N + j + 0]);
                __m256d c_i8_j0 = _mm256_load_pd(&C[(i + 8) * N + j + 0]);
                __m256d c_i9_j0 = _mm256_load_pd(&C[(i + 9) * N + j + 0]);
                __m256d c_i10_j0 = _mm256_load_pd(&C[(i + 10) * N + j + 0]);
                __m256d c_i11_j0 = _mm256_load_pd(&C[(i + 11) * N + j + 0]);
                __m256d c_i12_j0 = _mm256_load_pd(&C[(i + 12) * N + j + 0]);
                __m256d c_i13_j0 = _mm256_load_pd(&C[(i + 13) * N + j + 0]);
                __m256d c_i14_j0 = _mm256_load_pd(&C[(i + 14) * N + j + 0]);
                __m256d c_i15_j0 = _mm256_load_pd(&C[(i + 15) * N + j + 0]);
                

                __m256d b_k_j0 = _mm256_load_pd(&B[k * N + j + 0]);
                

                // op 1
                __m256d res_i0_j0 = VADD(a_i0_k, b_k_j0);
                __m256d res_i1_j0 = VADD(a_i1_k, b_k_j0);
                __m256d res_i2_j0 = VADD(a_i2_k, b_k_j0);
                __m256d res_i3_j0 = VADD(a_i3_k, b_k_j0);
                __m256d res_i4_j0 = VADD(a_i4_k, b_k_j0);
                __m256d res_i5_j0 = VADD(a_i5_k, b_k_j0);
                __m256d res_i6_j0 = VADD(a_i6_k, b_k_j0);
                __m256d res_i7_j0 = VADD(a_i7_k, b_k_j0);
                __m256d res_i8_j0 = VADD(a_i8_k, b_k_j0);
                __m256d res_i9_j0 = VADD(a_i9_k, b_k_j0);
                __m256d res_i10_j0 = VADD(a_i10_k, b_k_j0);
                __m256d res_i11_j0 = VADD(a_i11_k, b_k_j0);
                __m256d res_i12_j0 = VADD(a_i12_k, b_k_j0);
                __m256d res_i13_j0 = VADD(a_i13_k, b_k_j0);
                __m256d res_i14_j0 = VADD(a_i14_k, b_k_j0);
                __m256d res_i15_j0 = VADD(a_i15_k, b_k_j0);
                

                // op 2
                c_i0_j0 = VMIN(c_i0_j0, res_i0_j0);
                c_i1_j0 = VMIN(c_i1_j0, res_i1_j0);
                c_i2_j0 = VMIN(c_i2_j0, res_i2_j0);
                c_i3_j0 = VMIN(c_i3_j0, res_i3_j0);
                c_i4_j0 = VMIN(c_i4_j0, res_i4_j0);
                c_i5_j0 = VMIN(c_i5_j0, res_i5_j0);
                c_i6_j0 = VMIN(c_i6_j0, res_i6_j0);
                c_i7_j0 = VMIN(c_i7_j0, res_i7_j0);
                c_i8_j0 = VMIN(c_i8_j0, res_i8_j0);
                c_i9_j0 = VMIN(c_i9_j0, res_i9_j0);
                c_i10_j0 = VMIN(c_i10_j0, res_i10_j0);
                c_i11_j0 = VMIN(c_i11_j0, res_i11_j0);
                c_i12_j0 = VMIN(c_i12_j0, res_i12_j0);
                c_i13_j0 = VMIN(c_i13_j0, res_i13_j0);
                c_i14_j0 = VMIN(c_i14_j0, res_i14_j0);
                c_i15_j0 = VMIN(c_i15_j0, res_i15_j0);
                

                // store
                _mm256_store_pd(&C[(i + 0) * N + j + 0], c_i0_j0);
                _mm256_store_pd(&C[(i + 1) * N + j + 0], c_i1_j0);
                _mm256_store_pd(&C[(i + 2) * N + j + 0], c_i2_j0);
                _mm256_store_pd(&C[(i + 3) * N + j + 0], c_i3_j0);
                _mm256_store_pd(&C[(i + 4) * N + j + 0], c_i4_j0);
                _mm256_store_pd(&C[(i + 5) * N + j + 0], c_i5_j0);
                _mm256_store_pd(&C[(i + 6) * N + j + 0], c_i6_j0);
                _mm256_store_pd(&C[(i + 7) * N + j + 0], c_i7_j0);
                _mm256_store_pd(&C[(i + 8) * N + j + 0], c_i8_j0);
                _mm256_store_pd(&C[(i + 9) * N + j + 0], c_i9_j0);
                _mm256_store_pd(&C[(i + 10) * N + j + 0], c_i10_j0);
                _mm256_store_pd(&C[(i + 11) * N + j + 0], c_i11_j0);
                _mm256_store_pd(&C[(i + 12) * N + j + 0], c_i12_j0);
                _mm256_store_pd(&C[(i + 13) * N + j + 0], c_i13_j0);
                _mm256_store_pd(&C[(i + 14) * N + j + 0], c_i14_j0);
                _mm256_store_pd(&C[(i + 15) * N + j + 0], c_i15_j0);
                
            }
        }
    }
    return 0;
}

int floydWarshall(double *C, int N)
{
    return FWI(C, C, C, N);
}