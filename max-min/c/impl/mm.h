#ifndef FW_HEADER
#define FW_HEADER

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

/* Computes the paths that maximize the minimum edge weight and stores their lengths in C. N is the number of nodes.*/
int floydWarshall(double *C, int N);

#endif