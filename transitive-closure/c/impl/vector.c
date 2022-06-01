#include <immintrin.h>

#include "tc.h"

const int bytes_per_vec = 256 / 8;

/* vectorized version of the Floyd-Warshall Transitive Closure algorithm
 */
int floydWarshall(char *C, int N) {
    // compute constants / reused values
    int bpl = ceil(N / 8.0);  // bytes per matrix line

    for (int k = 0; k < N; k++) {
        for (int i = 0; i < N; i++) {
            // compute some indices for reuse
            int ibpl = i * bpl;
            int kbpl = k * bpl;

            // extract C[i, k] and generate according vector
            // if C[i, k] == 1, the vector (cik) is all ones
            // otherwise, the vector is all zeros
            char cikb = C[ibpl + k / 8];
            int byte_index = k % 8;
            char byte_mask = 1 << byte_index;

            char cik_byte = cikb & byte_mask ? 0xff : 0x00;
            __m256i cik = _mm256_set1_epi8(cik_byte);

            int j = 0;
            for (; j < bpl - (bytes_per_vec - 1); j += bytes_per_vec) {
                // load
                __m256i cij =
                  _mm256_loadu_si256((__m256i const *)&C[ibpl + j]);
                __m256i ckj =
                  _mm256_loadu_si256((__m256i const *)&C[kbpl + j]);

                // compute
                __m256i con = _mm256_and_si256(cik, ckj);
                __m256i res = _mm256_or_si256(con, cij);

                // store
                _mm256_storeu_si256((__m256i *)&C[ibpl + j], res);
            }
        }
    }
    return 0;
}
