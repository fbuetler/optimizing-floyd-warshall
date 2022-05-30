#include "mm.h"
#include <immintrin.h>

int floydWarshall(double *C, int N)
{
  for (int k = 0; k < N; k++)
  {
    for (int i = 0; i < N; i++)
    {
      __m256d cik = _mm256_broadcast_sd(&C[i * N + k]);
      for (int j = 0; j < N - 3; j += 4)
      {
        // load
        __m256d cij = _mm256_load_pd(&C[i * N + j]);
        __m256d ckj = _mm256_load_pd(&C[k * N + j]);

        // compute
        __m256d min = _mm256_min_pd(cik, ckj);
        __m256d res = _mm256_max_pd(cij, min);

        // store
        _mm256_store_pd(&C[i * N + j], res);
      }
    }
  }
  return 0;
}
