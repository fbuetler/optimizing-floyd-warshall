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

                // compute 1
                char and0_0 = ci0k & ckj0;
                char and0_1 = ci0k & ckj1;
                char and0_2 = ci0k & ckj2;
                char and0_3 = ci0k & ckj3;
                char and1_0 = ci1k & ckj0;
                char and1_1 = ci1k & ckj1;
                char and1_2 = ci1k & ckj2;
                char and1_3 = ci1k & ckj3;
                char and2_0 = ci2k & ckj0;
                char and2_1 = ci2k & ckj1;
                char and2_2 = ci2k & ckj2;
                char and2_3 = ci2k & ckj3;
                char and3_0 = ci3k & ckj0;
                char and3_1 = ci3k & ckj1;
                char and3_2 = ci3k & ckj2;
                char and3_3 = ci3k & ckj3;

                // compute 2
                char res0_0 = ci0j0 | and0_0;
                char res0_1 = ci0j1 | and0_1;
                char res0_2 = ci0j2 | and0_2;
                char res0_3 = ci0j3 | and0_3;
                char res1_0 = ci1j0 | and1_0;
                char res1_1 = ci1j1 | and1_1;
                char res1_2 = ci1j2 | and1_2;
                char res1_3 = ci1j3 | and1_3;
                char res2_0 = ci2j0 | and2_0;
                char res2_1 = ci2j1 | and2_1;
                char res2_2 = ci2j2 | and2_2;
                char res2_3 = ci2j3 | and2_3;
                char res3_0 = ci3j0 | and3_0;
                char res3_1 = ci3j1 | and3_1;
                char res3_2 = ci3j2 | and3_2;
                char res3_3 = ci3j3 | and3_3;

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

                // compute 1
                char and0 = cik & ckj0;
                char and1 = cik & ckj1;
                char and2 = cik & ckj2;
                char and3 = cik & ckj3;

                // compute 2
                char res0 = cij0 | and0;
                char res1 = cij1 | and1;
                char res2 = cij2 | and2;
                char res3 = cij3 | and3;

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
