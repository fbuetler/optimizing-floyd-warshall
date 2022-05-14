#include "sp.h"
#include <math.h>
#include <immintrin.h>

const int floats_per_vec = 256 / 32;

int floydWarshall(float *C, int N) {
  for (int k = 0; k < N; k++) {
    for (int i = 0; i < N; i++) {
      __m256 cik = _mm256_broadcast_ss(&C[i * N + k]);
      int j = 0;
      for (; j < N - (floats_per_vec - 1); j += floats_per_vec) {
        // load
        __m256 cij = _mm256_load_ps(&C[i * N + j]);
        __m256 ckj = _mm256_load_ps(&C[k * N + j]);

        // compute
        __m256 sum = _mm256_add_ps(cik, ckj);
        __m256 res = _mm256_min_ps(cij, sum);

        // store
        _mm256_storeu_ps(&C[i * N + j], res);
      }

      // only use part of a vector for the last remaining operations
      // compute mask
      __m256i cand_j = _mm256_set_epi32(j + 7, j + 6, j + 5, j + 4, j + 3, j + 2, j + 1, j);
      __m256i cmp = _mm256_set1_epi32(N);
      __m256i mask = _mm256_cmpgt_epi32(cmp, cand_j);

      // load
      __m256 cij = _mm256_maskload_ps(&C[i * N + j], mask);
      __m256 ckj = _mm256_maskload_ps(&C[k * N + j], mask);

      // compute
      __m256 sum = _mm256_add_ps(cik, ckj);
      __m256 res = _mm256_min_ps(cij, sum);

      // store
      _mm256_maskstore_ps(&C[i * N + j], mask, res);
    }
  }
  return 0;
}
