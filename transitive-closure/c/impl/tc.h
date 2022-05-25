#ifndef FW_HEADER
#define FW_HEADER

#include <math.h>

#define AND(X, Y) ((X & Y))
#define OR(X, Y) ((X | Y))

/* Computes the transitive closure of and stores it in C. N is the number of nodes.*/
int floydWarshall(char *C, int N);

#endif