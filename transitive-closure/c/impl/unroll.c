#include "tc.h"

/*
unoptimized version of the modified Floyd-Warshall transitive-closure algorithm
*/
int floydWarshall(char *C, int N)
{
    for (int k = 0; k < N; k++)
    {
        int i = 0;
        for (; i < N - 3; i += 4)
        {

            int j = 0;
            char ci0k = C[(i + 0) * N + k];
            char ci1k = C[(i + 1) * N + k];
            char ci2k = C[(i + 2) * N + k];
            char ci3k = C[(i + 3) * N + k];
            for (; j < N - 3; j += 4)
            {
                // load
                char ci0j0 = C[(i + 0) * N + j + 0];
                char ci0j1 = C[(i + 0) * N + j + 1];
                char ci0j2 = C[(i + 0) * N + j + 2];
                char ci0j3 = C[(i + 0) * N + j + 3];
                char ci1j0 = C[(i + 1) * N + j + 0];
                char ci1j1 = C[(i + 1) * N + j + 1];
                char ci1j2 = C[(i + 1) * N + j + 2];
                char ci1j3 = C[(i + 1) * N + j + 3];
                char ci2j0 = C[(i + 2) * N + j + 0];
                char ci2j1 = C[(i + 2) * N + j + 1];
                char ci2j2 = C[(i + 2) * N + j + 2];
                char ci2j3 = C[(i + 2) * N + j + 3];
                char ci3j0 = C[(i + 3) * N + j + 0];
                char ci3j1 = C[(i + 3) * N + j + 1];
                char ci3j2 = C[(i + 3) * N + j + 2];
                char ci3j3 = C[(i + 3) * N + j + 3];

                char ckj0 = C[k * N + j + 0];
                char ckj1 = C[k * N + j + 1];
                char ckj2 = C[k * N + j + 2];
                char ckj3 = C[k * N + j + 3];

                // compute
                char res00 = ci0j0 | (ci0k & ckj0);
                char res01 = ci0j1 | (ci0k & ckj1);
                char res02 = ci0j2 | (ci0k & ckj2);
                char res03 = ci0j3 | (ci0k & ckj3);
                char res10 = ci1j0 | (ci1k & ckj0);
                char res11 = ci1j1 | (ci1k & ckj1);
                char res12 = ci1j2 | (ci1k & ckj2);
                char res13 = ci1j3 | (ci1k & ckj3);
                char res20 = ci2j0 | (ci2k & ckj0);
                char res21 = ci2j1 | (ci2k & ckj1);
                char res22 = ci2j2 | (ci2k & ckj2);
                char res23 = ci2j3 | (ci2k & ckj3);
                char res30 = ci3j0 | (ci3k & ckj0);
                char res31 = ci3j1 | (ci3k & ckj1);
                char res32 = ci3j2 | (ci3k & ckj2);
                char res33 = ci3j3 | (ci3k & ckj3);

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
                C[(i + 0) * N + j] = C[(i + 0) * N + j] | (ci0k & C[k * N + j]);
                C[(i + 1) * N + j] = C[(i + 1) * N + j] | (ci1k & C[k * N + j]);
                C[(i + 2) * N + j] = C[(i + 2) * N + j] | (ci2k & C[k * N + j]);
                C[(i + 3) * N + j] = C[(i + 3) * N + j] | (ci3k & C[k * N + j]);
            }
        }

        // rest of i
        for (; i < N; i++)
        {

            int j = 0;
            char cik = C[i * N + k];
            for (; j < N - 3; j += 4)
            {
                // load
                char cij0 = C[i * N + j + 0];
                char cij1 = C[i * N + j + 1];
                char cij2 = C[i * N + j + 2];
                char cij3 = C[i * N + j + 3];

                char ckj0 = C[k * N + j + 0];
                char ckj1 = C[k * N + j + 1];
                char ckj2 = C[k * N + j + 2];
                char ckj3 = C[k * N + j + 3];

                // compute
                char res0 = cij0 | (cik & ckj0);
                char res1 = cij1 | (cik & ckj1);
                char res2 = cij2 | (cik & ckj2);
                char res3 = cij3 | (cik & ckj3);

                // store
                C[i * N + j + 0] = res0;
                C[i * N + j + 1] = res1;
                C[i * N + j + 2] = res2;
                C[i * N + j + 3] = res3;
            }
            for (; j < N; j++)
            {
                C[i * N + j] = C[i * N + j] | (cik & C[k * N + j]);
            }
        }
    }
}