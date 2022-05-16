#include "tc.h"
#include <math.h>
#include <immintrin.h>

const int bytes_per_vec = 256 / 8;

/* vectorized version of the Floyd-Warshall Transitive Closure algorithm
 * since vectors can only load / store to a precision of 32 bits (4 bytes),
 * the code to perform the remaining operations that don't fit neatly into a
 * vector is a bit ugly (sorry)
 */
int floydWarshall(char *C, int N) {
  int bpl = ceil(N / 8.0); // bytes per matrix line
  for (int k = 0; k < N; k++) {
    for (int i = 0; i < N; i++) {
      // extract C[i, k]
      char cikb = C[i * bpl + k / 8];
      int b_index = k % 8;
      char b_mask = 1 << b_index;

      char cik_byte = cikb & b_mask ? 0xff : 0x00;
      __m256i cik = _mm256_set1_epi8(cik_byte);

      int j = 0;
      for (; j < bpl - (bytes_per_vec - 1); j += bytes_per_vec) {
        // load
        __m256i cij = _mm256_loadu_si256((__m256i const *)&C[i * bpl + j]);
        __m256i ckj = _mm256_loadu_si256((__m256i const *)&C[k * bpl + j]);

        // compute
        __m256i con = _mm256_and_si256(cik, ckj);
        __m256i res = _mm256_or_si256(con, cij);

        // store
        _mm256_storeu_si256((__m256i *) &C[i * bpl + j], res);
      }

      // maybe do one set of 32-bit blocks
      // compute mask
      __m256i cand_j = _mm256_setr_epi32(j +  3, j +  7, j + 11, j + 15,
                                         j + 19, j + 23, j + 27, j + 31);
      __m256i cmp = _mm256_set1_epi32(bpl);
      __m256i mask = _mm256_cmpgt_epi32(cmp, cand_j);

      // load
      __m256i cij = _mm256_maskload_epi32((int const *)&C[i * bpl + j], mask);
      __m256i ckj = _mm256_maskload_epi32((int const *)&C[k * bpl + j], mask);

      // compute
      __m256i con = _mm256_and_si256(cik, ckj);
      __m256i res = _mm256_or_si256(con, cij);

      // store
      _mm256_maskstore_epi32((int *) &C[i * bpl + j], mask, res);

      // increase j for the bytes that were just handled
      j = bpl - (bpl % 4);

      // since we don't have AVX-512, we cannot perform loads / stores to byte-precision,
      // so the remaining bytes (at most 3) have to be computed boringly
      for (; j < bpl; j++) {
        C[i * bpl + j] = C[i * bpl + j] | (cik_byte & C[k * bpl + j]);
      }
    }
  }
  return 0;
}
