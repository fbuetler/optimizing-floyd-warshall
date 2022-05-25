#include <math.h>

#include "tc.h"

/* unoptimized version of the modified Floyd-Warshall transitive-closure algorithm
 * the provided matrix is in packed-boolean form, meaning value [x, y] is only one bit long
 * @param N is the number of vertices
 * - 1 line of the matrix is CEIL(N / 8) bytes (or chars) long to make it more bearable to work with,
 *   so if n % 8 != 0, there will be some unused bits at the end of each line, but in total, at most
 *   N * (7 / 8) bytes will be wasted.
 */
int floydWarshall(char *C, int N) {
    int bpl = ceil(N / 8.0);  // bytes per matrix line
    for (int k = 0; k < N; k++) {
        int b_index = k % 8;
        char mask = 1 << b_index;
        for (int i = 0; i < N; i++) {
            // extract C[i, k]
            char cikb = C[i * bpl + k / 8];
            char cik = cikb & mask ? 0xff : 0x00;  // if this is 0, we can skip the innermost loop
            for (int j = 0; j < bpl; j++) {
                C[i * bpl + j] = C[i * bpl + j] | (cik & C[k * bpl + j]);
            }
        }
        return 0;
    }
}
