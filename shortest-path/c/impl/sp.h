#ifndef FW_HEADER
#define FW_HEADER

#define SUBM(X, u, v, RW, TH, TW) ((X) + (u) * (RW) * (TH) + (v) * (TW))

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define ADD(X, Y) ((X + Y))

#define VMIN(X, Y) (_mm256_min_pd((X), (Y)))
#define VADD(X, Y) (_mm256_add_pd((X), (Y)))

/* Computes the shortest paths between any pair of vertices and stores their lengths in C. N is the number of nodes.*/
int floydWarshall(double *C, int N);

#endif
