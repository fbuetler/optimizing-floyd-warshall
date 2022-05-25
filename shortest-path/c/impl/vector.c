#include "sp.h"
#include <immintrin.h>

const int doubles_per_vec = 256 / 64;

int floydWarshall(double *C, int N) {
  // compute mask for the last vector of each loop
  __m256i mask_indices = _mm256_set_epi64x(3, 2, 1, 0);
  __m256i mask_limit = _mm256_set1_epi64x(N % doubles_per_vec);
  __m256i mask = _mm256_cmpgt_epi64(mask_limit, mask_indices);

  for (int k = 0; k < N; k++) {
    for (int i = 0; i < N; i++) {
      __m256d cik = _mm256_broadcast_sd(&C[i * N + k]);
      int j = 0;
      for (; j < N - (doubles_per_vec - 1); j += doubles_per_vec) {
        // load
        __m256d cij = _mm256_load_pd(&C[i * N + j]);
        __m256d ckj = _mm256_load_pd(&C[k * N + j]);

        // compute
        __m256d sum = _mm256_add_pd(cik, ckj);
        __m256d res = _mm256_min_pd(cij, sum);

        // store
        _mm256_storeu_pd(&C[i * N + j], res);
      }

      // load
      __m256d cij = _mm256_maskload_pd(&C[i * N + j], mask);
      __m256d ckj = _mm256_maskload_pd(&C[k * N + j], mask);

      // compute
      __m256d sum = _mm256_add_pd(cik, ckj);
      __m256d res = _mm256_min_pd(cij, sum);

      // store
      _mm256_maskstore_pd(&C[i * N + j], mask, res);
    }
  }
  return 0;
}
