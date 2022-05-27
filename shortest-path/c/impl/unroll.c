#include "sp.h"

/* minimally optimized version of floyd-warshall:
 * - the two innermost loops are unrolled by a factor of 4
 * - the constant value C[i0..3 * N + k] is only loaded once before the innermost loop
 */
int floydWarshall(double *C, int N)
{
    for (int k = 0; k < N; k++)
    {
        int i = 0;
        for (; i < N - 3; i += 4)
        {
            int j = 0;
            double ci0k = C[(i + 0) * N + k];
            double ci1k = C[(i + 1) * N + k];
            double ci2k = C[(i + 2) * N + k];
            double ci3k = C[(i + 3) * N + k];
            for (; j < N - 3; j += 4)
            {
                // load
                double ci0j0 = C[(i + 0) * N + j + 0];
                double ci0j1 = C[(i + 0) * N + j + 1];
                double ci0j2 = C[(i + 0) * N + j + 2];
                double ci0j3 = C[(i + 0) * N + j + 3];
                double ci1j0 = C[(i + 1) * N + j + 0];
                double ci1j1 = C[(i + 1) * N + j + 1];
                double ci1j2 = C[(i + 1) * N + j + 2];
                double ci1j3 = C[(i + 1) * N + j + 3];
                double ci2j0 = C[(i + 2) * N + j + 0];
                double ci2j1 = C[(i + 2) * N + j + 1];
                double ci2j2 = C[(i + 2) * N + j + 2];
                double ci2j3 = C[(i + 2) * N + j + 3];
                double ci3j0 = C[(i + 3) * N + j + 0];
                double ci3j1 = C[(i + 3) * N + j + 1];
                double ci3j2 = C[(i + 3) * N + j + 2];
                double ci3j3 = C[(i + 3) * N + j + 3];

                double ckj0 = C[k * N + j + 0];
                double ckj1 = C[k * N + j + 1];
                double ckj2 = C[k * N + j + 2];
                double ckj3 = C[k * N + j + 3];

                // compute 1
                double sum0_0 = ci0k + ckj0;
                double sum0_1 = ci0k + ckj1;
                double sum0_2 = ci0k + ckj2;
                double sum0_3 = ci0k + ckj3;
                double sum1_0 = ci1k + ckj0;
                double sum1_1 = ci1k + ckj1;
                double sum1_2 = ci1k + ckj2;
                double sum1_3 = ci1k + ckj3;
                double sum2_0 = ci2k + ckj0;
                double sum2_1 = ci2k + ckj1;
                double sum2_2 = ci2k + ckj2;
                double sum2_3 = ci2k + ckj3;
                double sum3_0 = ci3k + ckj0;
                double sum3_1 = ci3k + ckj1;
                double sum3_2 = ci3k + ckj2;
                double sum3_3 = ci3k + ckj3;

                // compute 2
                double res0_0 = MIN(ci0j0, sum0_0);
                double res0_1 = MIN(ci0j1, sum0_1);
                double res0_2 = MIN(ci0j2, sum0_2);
                double res0_3 = MIN(ci0j3, sum0_3);
                double res1_0 = MIN(ci1j0, sum1_0);
                double res1_1 = MIN(ci1j1, sum1_1);
                double res1_2 = MIN(ci1j2, sum1_2);
                double res1_3 = MIN(ci1j3, sum1_3);
                double res2_0 = MIN(ci2j0, sum2_0);
                double res2_1 = MIN(ci2j1, sum2_1);
                double res2_2 = MIN(ci2j2, sum2_2);
                double res2_3 = MIN(ci2j3, sum2_3);
                double res3_0 = MIN(ci3j0, sum3_0);
                double res3_1 = MIN(ci3j1, sum3_1);
                double res3_2 = MIN(ci3j2, sum3_2);
                double res3_3 = MIN(ci3j3, sum3_3);

                // store
                C[(i + 0) * N + j + 0] = res0_0;
                C[(i + 0) * N + j + 1] = res0_1;
                C[(i + 0) * N + j + 2] = res0_2;
                C[(i + 0) * N + j + 3] = res0_3;
                C[(i + 1) * N + j + 0] = res1_0;
                C[(i + 1) * N + j + 1] = res1_1;
                C[(i + 1) * N + j + 2] = res1_2;
                C[(i + 1) * N + j + 3] = res1_3;
                C[(i + 2) * N + j + 0] = res2_0;
                C[(i + 2) * N + j + 1] = res2_1;
                C[(i + 2) * N + j + 2] = res2_2;
                C[(i + 2) * N + j + 3] = res2_3;
                C[(i + 3) * N + j + 0] = res3_0;
                C[(i + 3) * N + j + 1] = res3_1;
                C[(i + 3) * N + j + 2] = res3_2;
                C[(i + 3) * N + j + 3] = res3_3;
            }
            for (; j < N; j++)
            {
                C[(i + 0) * N + j] = MIN(C[(i + 0) * N + j], ci0k + C[k * N + j]);
                C[(i + 1) * N + j] = MIN(C[(i + 1) * N + j], ci1k + C[k * N + j]);
                C[(i + 2) * N + j] = MIN(C[(i + 2) * N + j], ci2k + C[k * N + j]);
                C[(i + 3) * N + j] = MIN(C[(i + 3) * N + j], ci3k + C[k * N + j]);
            }
        }

        // rest of i
        for (; i < N; i++)
        {
            int j = 0;
            double cik = C[i * N + k];
            for (; j < N - 3; j += 4)
            {
                // load
                double cij0 = C[i * N + j + 0];
                double cij1 = C[i * N + j + 1];
                double cij2 = C[i * N + j + 2];
                double cij3 = C[i * N + j + 3];

                double ckj0 = C[k * N + j + 0];
                double ckj1 = C[k * N + j + 1];
                double ckj2 = C[k * N + j + 2];
                double ckj3 = C[k * N + j + 3];

                // compute 1
                double sum0 = cik + ckj0;
                double sum1 = cik + ckj1;
                double sum2 = cik + ckj2;
                double sum3 = cik + ckj3;

                // compute 2
                double res0 = MIN(cij0, sum0);
                double res1 = MIN(cij1, sum1);
                double res2 = MIN(cij2, sum2);
                double res3 = MIN(cij3, sum3);

                // store
                C[i * N + j + 0] = res0;
                C[i * N + j + 1] = res1;
                C[i * N + j + 2] = res2;
                C[i * N + j + 3] = res3;
            }
            for (; j < N; j++)
            {
                C[i * N + j] = MIN(C[i * N + j], cik + C[k * N + j]);
            }
        }
    }
    return 0;
}
