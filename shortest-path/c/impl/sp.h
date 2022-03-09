#ifndef FW_HEADER
#define FW_HEADER

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

/* Computes the shortest paths between any pair of vertices and stores their lengths in C. N is the number of nodes.*/
void floydWarshall(float *C, int N);

#endif