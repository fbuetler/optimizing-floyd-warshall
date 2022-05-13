#ifndef FW_HEADER
#define FW_HEADER

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

/* Computes the transitive closure of and stores it in C. N is the number of nodes.*/
int floydWarshall(char *C, int N);

#endif