#ifndef FW_HEADER
#define FW_HEADER

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

/* Computes all shortest paths between any pair of vertices by applying the FW algorithm */
void floydWarshall(float **C, int N);

#endif