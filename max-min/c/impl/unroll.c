#include "mm.h"

/*
unoptimized version of the modified Floyd-Warshall maximin algorithm
*/
int floydWarshall(float *C, int N)
{
    for (int k = 0; k < N; k++)
    {
        int i = 0;
        for (; i < N - 3; i += 4)
        {

            int j = 0;
            float ci0k = C[(i + 0) * N + k];
            float ci1k = C[(i + 1) * N + k];
            float ci2k = C[(i + 2) * N + k];
            float ci3k = C[(i + 3) * N + k];
            for (; j < N - 3; j += 4)
            {
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
                float res00 = MAX(ci0j0, MIN(ci0k, ckj0));
                float res01 = MAX(ci0j1, MIN(ci0k, ckj1));
                float res02 = MAX(ci0j2, MIN(ci0k, ckj2));
                float res03 = MAX(ci0j3, MIN(ci0k, ckj3));
                float res10 = MAX(ci1j0, MIN(ci1k, ckj0));
                float res11 = MAX(ci1j1, MIN(ci1k, ckj1));
                float res12 = MAX(ci1j2, MIN(ci1k, ckj2));
                float res13 = MAX(ci1j3, MIN(ci1k, ckj3));
                float res20 = MAX(ci2j0, MIN(ci2k, ckj0));
                float res21 = MAX(ci2j1, MIN(ci2k, ckj1));
                float res22 = MAX(ci2j2, MIN(ci2k, ckj2));
                float res23 = MAX(ci2j3, MIN(ci2k, ckj3));
                float res30 = MAX(ci3j0, MIN(ci3k, ckj0));
                float res31 = MAX(ci3j1, MIN(ci3k, ckj1));
                float res32 = MAX(ci3j2, MIN(ci3k, ckj2));
                float res33 = MAX(ci3j3, MIN(ci3k, ckj3));

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
            float cik = C[i * N + k];
            for (; j < N - 3; j += 4)
            {
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
                float res0 = MAX(cij0, MIN(cik, ckj0));
                float res1 = MAX(cij1, MIN(cik, ckj1));
                float res2 = MAX(cij2, MIN(cik, ckj2));
                float res3 = MAX(cij3, MIN(cik, ckj3));

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