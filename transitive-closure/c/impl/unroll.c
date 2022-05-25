#include <math.h>

#include "tc.h"

/*
unoptimized version of the modified Floyd-Warshall transitive-closure algorithm
*/
int floydWarshall(char *C, int N)
{
    int bpl = ceil(N / 8.0); // bytes per matrix line

    for (int k = 0; k < N; k++)
    {
        int i = 0;
        for (; i < N - 3; i += 4)
        {
            // extract C[i, k]
            int j = 0;
            char ci0k = C[(i + 0) * bpl + k / 8] & (1 << (k % 8)) ? 0xff : 0x00; // if this is 0, we can skip the innermost loop
            char ci1k = C[(i + 1) * bpl + k / 8] & (1 << (k % 8)) ? 0xff : 0x00; // if this is 0, we can skip the innermost loop
            char ci2k = C[(i + 2) * bpl + k / 8] & (1 << (k % 8)) ? 0xff : 0x00; // if this is 0, we can skip the innermost loop
            char ci3k = C[(i + 3) * bpl + k / 8] & (1 << (k % 8)) ? 0xff : 0x00; // if this is 0, we can skip the innermost loop

            for (; j < bpl - 3; j += 4)
            {
                // load
                char ci0j0 = C[(i + 0) * bpl + j + 0];
                char ci0j1 = C[(i + 0) * bpl + j + 1];
                char ci0j2 = C[(i + 0) * bpl + j + 2];
                char ci0j3 = C[(i + 0) * bpl + j + 3];
                char ci1j0 = C[(i + 1) * bpl + j + 0];
                char ci1j1 = C[(i + 1) * bpl + j + 1];
                char ci1j2 = C[(i + 1) * bpl + j + 2];
                char ci1j3 = C[(i + 1) * bpl + j + 3];
                char ci2j0 = C[(i + 2) * bpl + j + 0];
                char ci2j1 = C[(i + 2) * bpl + j + 1];
                char ci2j2 = C[(i + 2) * bpl + j + 2];
                char ci2j3 = C[(i + 2) * bpl + j + 3];
                char ci3j0 = C[(i + 3) * bpl + j + 0];
                char ci3j1 = C[(i + 3) * bpl + j + 1];
                char ci3j2 = C[(i + 3) * bpl + j + 2];
                char ci3j3 = C[(i + 3) * bpl + j + 3];

                char ckj0 = C[k * bpl + j + 0];
                char ckj1 = C[k * bpl + j + 1];
                char ckj2 = C[k * bpl + j + 2];
                char ckj3 = C[k * bpl + j + 3];

                // compute
                char res0_0 = ci0j0 | (ci0k & ckj0);
                char res0_1 = ci0j1 | (ci0k & ckj1);
                char res0_2 = ci0j2 | (ci0k & ckj2);
                char res0_3 = ci0j3 | (ci0k & ckj3);
                char res1_0 = ci1j0 | (ci1k & ckj0);
                char res1_1 = ci1j1 | (ci1k & ckj1);
                char res1_2 = ci1j2 | (ci1k & ckj2);
                char res1_3 = ci1j3 | (ci1k & ckj3);
                char res2_0 = ci2j0 | (ci2k & ckj0);
                char res2_1 = ci2j1 | (ci2k & ckj1);
                char res2_2 = ci2j2 | (ci2k & ckj2);
                char res2_3 = ci2j3 | (ci2k & ckj3);
                char res3_0 = ci3j0 | (ci3k & ckj0);
                char res3_1 = ci3j1 | (ci3k & ckj1);
                char res3_2 = ci3j2 | (ci3k & ckj2);
                char res3_3 = ci3j3 | (ci3k & ckj3);

                // store
                C[(i + 0) * bpl + j + 0] = res0_0;
                C[(i + 0) * bpl + j + 1] = res0_1;
                C[(i + 0) * bpl + j + 2] = res0_2;
                C[(i + 0) * bpl + j + 3] = res0_3;
                C[(i + 1) * bpl + j + 0] = res1_0;
                C[(i + 1) * bpl + j + 1] = res1_1;
                C[(i + 1) * bpl + j + 2] = res1_2;
                C[(i + 1) * bpl + j + 3] = res1_3;
                C[(i + 2) * bpl + j + 0] = res2_0;
                C[(i + 2) * bpl + j + 1] = res2_1;
                C[(i + 2) * bpl + j + 2] = res2_2;
                C[(i + 2) * bpl + j + 3] = res2_3;
                C[(i + 3) * bpl + j + 0] = res3_0;
                C[(i + 3) * bpl + j + 1] = res3_1;
                C[(i + 3) * bpl + j + 2] = res3_2;
                C[(i + 3) * bpl + j + 3] = res3_3;
            }
            for (; j < bpl; j++)
            {
                C[(i + 0) * bpl + j] = C[(i + 0) * bpl + j] | (ci0k & C[k * bpl + j]);
                C[(i + 1) * bpl + j] = C[(i + 1) * bpl + j] | (ci1k & C[k * bpl + j]);
                C[(i + 2) * bpl + j] = C[(i + 2) * bpl + j] | (ci2k & C[k * bpl + j]);
                C[(i + 3) * bpl + j] = C[(i + 3) * bpl + j] | (ci3k & C[k * bpl + j]);
            }
        }

        // rest of i
        for (; i < N; i++)
        {
            int j = 0;
            char cik = C[i * bpl + k / 8] & (1 << (k % 8)) ? 0xff : 0x00; // if this is 0, we can skip the innermost loop
            for (; j < bpl - 3; j += 4)
            {
                // load
                char cij0 = C[i * bpl + j + 0];
                char cij1 = C[i * bpl + j + 1];
                char cij2 = C[i * bpl + j + 2];
                char cij3 = C[i * bpl + j + 3];

                char ckj0 = C[k * bpl + j + 0];
                char ckj1 = C[k * bpl + j + 1];
                char ckj2 = C[k * bpl + j + 2];
                char ckj3 = C[k * bpl + j + 3];

                // compute
                char res0 = cij0 | (cik & ckj0);
                char res1 = cij1 | (cik & ckj1);
                char res2 = cij2 | (cik & ckj2);
                char res3 = cij3 | (cik & ckj3);

                // store
                C[i * bpl + j + 0] = res0;
                C[i * bpl + j + 1] = res1;
                C[i * bpl + j + 2] = res2;
                C[i * bpl + j + 3] = res3;
            }
            for (; j < bpl; j++)
            {
                C[i * bpl + j] = C[i * bpl + j] | (cik & C[k * bpl + j]);
            }
        }
    }
    return 0;
}