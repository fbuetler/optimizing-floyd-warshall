#include "sp.h"

/* minimally optimized version of floyd-warshall:
 * - the two innermost loops are unrolled by a factor of 4
 * - the constant value C[i0..3 * N + k] is only loaded once before the innermost loop
 */
int floydWarshall(float *C, int N) {
    for (int k = 0; k < N; k++) {
        int i = 0;
        for (; i < N - 3; i += 4) {
            int j = 0;
            float ci0k = C[(i + 0) * N + k];
            float ci1k = C[(i + 1) * N + k];
            float ci2k = C[(i + 2) * N + k];
            float ci3k = C[(i + 3) * N + k];
            for (; j < N - 3; j += 4) {
                // load
                float ci0j0 = C[(i + 0) * N + j + 0];
                float ci0j1 = C[(i + 0) * N + j + 1];
                float ci0j2 = C[(i + 0) * N + j + 2];
                float ci0j3 = C[(i + 0) * N + j + 3];
                float ci1j0 = C[(i + 1) * N + j + 0];
                float ci1j1 = C[(i + 1) * N + j + 1];
                float ci1j2 = C[(i + 1) * N + j + 2];
                float ci1j3 = C[(i + 1) * N + j + 3];
                float ci2j0 = C[(i + 2) * N + j + 0];
                float ci2j1 = C[(i + 2) * N + j + 1];
                float ci2j2 = C[(i + 2) * N + j + 2];
                float ci2j3 = C[(i + 2) * N + j + 3];
                float ci3j0 = C[(i + 3) * N + j + 0];
                float ci3j1 = C[(i + 3) * N + j + 1];
                float ci3j2 = C[(i + 3) * N + j + 2];
                float ci3j3 = C[(i + 3) * N + j + 3];

                float ckj0 = C[k * N + j + 0];
                float ckj1 = C[k * N + j + 1];
                float ckj2 = C[k * N + j + 2];
                float ckj3 = C[k * N + j + 3];

                // compute
                float res00 = MIN(ci0j0, ci0k + ckj0);
                float res01 = MIN(ci0j1, ci0k + ckj1);
                float res02 = MIN(ci0j2, ci0k + ckj2);
                float res03 = MIN(ci0j3, ci0k + ckj3);
                float res10 = MIN(ci1j0, ci1k + ckj0);
                float res11 = MIN(ci1j1, ci1k + ckj1);
                float res12 = MIN(ci1j2, ci1k + ckj2);
                float res13 = MIN(ci1j3, ci1k + ckj3);
                float res20 = MIN(ci2j0, ci2k + ckj0);
                float res21 = MIN(ci2j1, ci2k + ckj1);
                float res22 = MIN(ci2j2, ci2k + ckj2);
                float res23 = MIN(ci2j3, ci2k + ckj3);
                float res30 = MIN(ci3j0, ci3k + ckj0);
                float res31 = MIN(ci3j1, ci3k + ckj1);
                float res32 = MIN(ci3j2, ci3k + ckj2);
                float res33 = MIN(ci3j3, ci3k + ckj3);

                // store
                C[(i + 0) * N + j + 0] = res00;
                C[(i + 0) * N + j + 1] = res01;
                C[(i + 0) * N + j + 2] = res02;
                C[(i + 0) * N + j + 3] = res03;
                C[(i + 1) * N + j + 0] = res10;
                C[(i + 1) * N + j + 1] = res11;
                C[(i + 1) * N + j + 2] = res12;
                C[(i + 1) * N + j + 3] = res13;
                C[(i + 2) * N + j + 0] = res20;
                C[(i + 2) * N + j + 1] = res21;
                C[(i + 2) * N + j + 2] = res22;
                C[(i + 2) * N + j + 3] = res23;
                C[(i + 3) * N + j + 0] = res30;
                C[(i + 3) * N + j + 1] = res31;
                C[(i + 3) * N + j + 2] = res32;
                C[(i + 3) * N + j + 3] = res33;
            }
            for (; j < N; j++) {
                C[(i + 0) * N + j] = MIN(C[(i + 0) * N + j], ci0k + C[k * N + j]);
                C[(i + 1) * N + j] = MIN(C[(i + 1) * N + j], ci1k + C[k * N + j]);
                C[(i + 2) * N + j] = MIN(C[(i + 2) * N + j], ci2k + C[k * N + j]);
                C[(i + 3) * N + j] = MIN(C[(i + 3) * N + j], ci3k + C[k * N + j]);
            }
        }

        // rest of i
        for (; i < N; i++) {
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
    return 0;
}
