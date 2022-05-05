#include <stdio.h>
#include <iostream>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/transitive_closure.hpp>

using namespace std;

// types
typedef boost::adjacency_matrix<boost::directedS> Graph;
typedef boost::adjacency_list < boost::vecS, boost::vecS, boost::directedS, boost::no_property > Graph2;

// utility functions to print matrices in both formats
void printMatrix(Graph &G, long int N) {
  for (long int i = 0; i < N; i++) {
    for (long int j = 0; j < N; j++) {
      if (boost::edge(i, j, G).second) {
        fprintf(stderr, "1, ");
      } else {
        fprintf(stderr, " , ");
      }
    }
    fprintf(stderr, "\n");
  }
}

void printMatrix2(Graph2 &G, long int N) {
  for (long int i = 0; i < N; i++) {
    for (long int j = 0; j < N; j++) {
      if (boost::edge(i, j, G).second) {
        fprintf(stderr, "1, ");
      } else {
        fprintf(stderr, " , ");
      }
    }
    fprintf(stderr, "\n");
  }
}

// write matrix in output-form to file
void output_matrix(char *filename, Graph2 &G, long int N) {
  fprintf(stderr, "outputting transitive closure matrix to %s...\n", filename);
  FILE *output_f = fopen(filename, "w+");
  fprintf(output_f, "%d\n", N);
  for (long int i = 0; i < N; i++) {
    for (long int j = 0; j < N; j++) {
      if (boost::edge(i, j, G).second) {
        fprintf(output_f, "1");
      } else {
        fprintf(output_f, "0");
      }
      if (j < N - 1) {
        fputc(',', output_f);
      } else {
        fputc('\n', output_f);
      }
    }
  }
  fclose(output_f);
}

// Main function: read matrix, run algorithm, write result
int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "incorrect number of arguments\n");
    fprintf(stderr, "call as: ./main input_filename output_filename\n");
    return -1;
  }
  cout << argv[1] << endl;
  FILE *input_f = fopen(argv[1], "r");

  // read graph
  int n; // num nodes
  if (fscanf(input_f, "%d ", &n) < 1) {
    fprintf(stderr, "malformed input: couldn't match number N of vertices\n");
    return -1;
  }

  long int N = n;

  Graph G(N); // input graph
  Graph2 result; // graph to write result to
  for (long unsigned int i = 0; i < N; i++) {
    char inputValue[100];
    for (long unsigned int j = 0; j < N; j++) {
      int numValues = fscanf(input_f, "%[^,\n]s", inputValue);
      if (numValues == 1) {
        boost::add_edge(i, j, G);
      }
      fgetc(input_f); // skip ',' or '\n'
    }
  }

  // printMatrix(G, N);
  boost::transitive_closure(G, result);
  // printMatrix2(result, N);

  output_matrix(argv[2], result, N);
  return 0;
}
