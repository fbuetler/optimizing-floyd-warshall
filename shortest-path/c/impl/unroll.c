#include "sp.h"

/* minimally optimized version of floyd-warshall:
 * - the innermost loop is unrolled by a factor of 4
 * - the constant value C[i * N + k] is only acessed once before the innermost loop
*/
void floydWarshall(float *C, int N) {
  for (int k = 0; k < N; k++) {
    for (int i = 0; i < N; i++) {

      int j = 0;
      float cik = C[i * N + k];
      for (; j < N - 3; j += 4) {
        // load
        float cij0 = C[i * N + j + 0];
        float cij1 = C[i * N + j + 1];
        float cij2 = C[i * N + j + 2];
        float cij3 = C[i * N + j + 3];

        float ckj0 = C[k * N + j + 0];
        float ckj1 = C[k * N + j + 1];
        float ckj2 = C[k * N + j + 2];
        float ckj3 = C[k * N + j + 3];

        // compute
        float res0 = MIN(cij0, cik + ckj0);
        float res1 = MIN(cij1, cik + ckj1);
        float res2 = MIN(cij2, cik + ckj2);
        float res3 = MIN(cij3, cik + ckj3);

        // store
        C[i * N + j + 0] = res0;
        C[i * N + j + 1] = res1;
        C[i * N + j + 2] = res2;
        C[i * N + j + 3] = res3;
      }
      for (; j < N; j++) {
        C[i * N + j] = MIN(C[i * N + j], cik + C[k * N + j]);
      }
    }
  }
}
