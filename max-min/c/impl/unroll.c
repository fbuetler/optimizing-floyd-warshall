#include "mm.h"

/*
 * manually unrolled version of the modified Floyd-Warshall maximin algorithm
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
                double min0_0 = MIN(ci0k, ckj0);
                double min0_1 = MIN(ci0k, ckj1);
                double min0_2 = MIN(ci0k, ckj2);
                double min0_3 = MIN(ci0k, ckj3);
                double min1_0 = MIN(ci1k, ckj0);
                double min1_1 = MIN(ci1k, ckj1);
                double min1_2 = MIN(ci1k, ckj2);
                double min1_3 = MIN(ci1k, ckj3);
                double min2_0 = MIN(ci2k, ckj0);
                double min2_1 = MIN(ci2k, ckj1);
                double min2_2 = MIN(ci2k, ckj2);
                double min2_3 = MIN(ci2k, ckj3);
                double min3_0 = MIN(ci3k, ckj0);
                double min3_1 = MIN(ci3k, ckj1);
                double min3_2 = MIN(ci3k, ckj2);
                double min3_3 = MIN(ci3k, ckj3);

                // compute 2
                double res0_0 = MAX(ci0j0, min0_0);
                double res0_1 = MAX(ci0j1, min0_1);
                double res0_2 = MAX(ci0j2, min0_2);
                double res0_3 = MAX(ci0j3, min0_3);
                double res1_0 = MAX(ci1j0, min1_0);
                double res1_1 = MAX(ci1j1, min1_1);
                double res1_2 = MAX(ci1j2, min1_2);
                double res1_3 = MAX(ci1j3, min1_3);
                double res2_0 = MAX(ci2j0, min2_0);
                double res2_1 = MAX(ci2j1, min2_1);
                double res2_2 = MAX(ci2j2, min2_2);
                double res2_3 = MAX(ci2j3, min2_3);
                double res3_0 = MAX(ci3j0, min3_0);
                double res3_1 = MAX(ci3j1, min3_1);
                double res3_2 = MAX(ci3j2, min3_2);
                double res3_3 = MAX(ci3j3, min3_3);

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
        }
    }
    return 0;
}
