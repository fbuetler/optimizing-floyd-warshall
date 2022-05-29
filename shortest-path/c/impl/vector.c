#include "sp.h"
#include <immintrin.h>

const int doubles_per_vec = 256 / 64;

int floydWarshall(double *C, int N) {
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
        _mm256_store_pd(&C[i * N + j], res);
      }
    }
  }
  return 0;
}
