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
      for (; j < N; j++) {
        C[i * N + j] = MIN(C[i * N + j], C[i * N + k] + C[k * N + j]);
      }
    }
  }
  return 0;
}
