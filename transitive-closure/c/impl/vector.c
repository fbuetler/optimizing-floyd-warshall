#include "tc.h"
#include <math.h>
#include <immintrin.h>

const int bytes_per_vec = 256 / 8;

/* unoptimized version of the modified Floyd-Warshall transitive-closure algorithm
 * the provided matrix is in packed-boolean form, meaning value [x, y] is only one bit long
 * @param N is the number of vertices
 * - 1 line of the matrix is CEIL(N / 8) bytes (or chars) long to make it more bearable to work with,
 *   so if n % 8 != 0, there will be some unused bits at the end of each line, but in total, at most
 *   N * (7 / 8) bytes will be wasted.
 */
int floydWarshall(char *C, int N) {
  int bpl = ceil(N / 8.0); // bytes per matrix line
  for (int k = 0; k < N; k++) {
    for (int i = 0; i < N; i++) {
      // extract C[i, k]
      char cikb = C[i * bpl + k / 8];
      int b_index = k % 8;
      char mask = 1 << b_index;

      char cik_byte = cikb & mask ? 0xff : 0x00;
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

      // TODO: do this with vectors too
      for (; j < bpl; j++) {
        C[i * bpl + j] = C[i * bpl + j] | (cik_byte & C[k * bpl + j]);
      }
    }
  }
  return 0;
}
