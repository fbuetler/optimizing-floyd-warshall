#include "mm.h"

/*
unoptimized version of the modified Floyd-Warshall maximin algorithm
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

                // compute
                double res0_0 = MAX(ci0j0, MIN(ci0k, ckj0));
                double res0_1 = MAX(ci0j1, MIN(ci0k, ckj1));
                double res0_2 = MAX(ci0j2, MIN(ci0k, ckj2));
                double res0_3 = MAX(ci0j3, MIN(ci0k, ckj3));
                double res1_0 = MAX(ci1j0, MIN(ci1k, ckj0));
                double res1_1 = MAX(ci1j1, MIN(ci1k, ckj1));
                double res1_2 = MAX(ci1j2, MIN(ci1k, ckj2));
                double res1_3 = MAX(ci1j3, MIN(ci1k, ckj3));
                double res2_0 = MAX(ci2j0, MIN(ci2k, ckj0));
                double res2_1 = MAX(ci2j1, MIN(ci2k, ckj1));
                double res2_2 = MAX(ci2j2, MIN(ci2k, ckj2));
                double res2_3 = MAX(ci2j3, MIN(ci2k, ckj3));
                double res3_0 = MAX(ci3j0, MIN(ci3k, ckj0));
                double res3_1 = MAX(ci3j1, MIN(ci3k, ckj1));
                double res3_2 = MAX(ci3j2, MIN(ci3k, ckj2));
                double res3_3 = MAX(ci3j3, MIN(ci3k, ckj3));

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
                C[(i + 0) * N + j] = MAX(C[(i + 0) * N + j], MIN(ci0k, C[k * N + j]));
                C[(i + 1) * N + j] = MAX(C[(i + 1) * N + j], MIN(ci1k, C[k * N + j]));
                C[(i + 2) * N + j] = MAX(C[(i + 2) * N + j], MIN(ci2k, C[k * N + j]));
                C[(i + 3) * N + j] = MAX(C[(i + 3) * N + j], MIN(ci3k, C[k * N + j]));
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

                // compute
                double res0 = MAX(cij0, MIN(cik, ckj0));
                double res1 = MAX(cij1, MIN(cik, ckj1));
                double res2 = MAX(cij2, MIN(cik, ckj2));
                double res3 = MAX(cij3, MIN(cik, ckj3));

                // store
                C[i * N + j + 0] = res0;
                C[i * N + j + 1] = res1;
                C[i * N + j + 2] = res2;
                C[i * N + j + 3] = res3;
            }
            for (; j < N; j++)
            {
                C[i * N + j] = MAX(C[i * N + j], MIN(cik, C[k * N + j]));
            }
        }
    }
    return 0;
}