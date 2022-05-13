#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/property_maps/constant_property_map.hpp>
#include <boost/graph/floyd_warshall_shortest.hpp>
#include <boost/graph/directed_graph.hpp>
#include <boost/graph/exterior_property.hpp>

// types
// graph type
typedef boost::property<boost::edge_weight_t, double> edge_weight_property;
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, 
                              boost::no_property, edge_weight_property> Graph;
typedef boost::property_map<Graph, boost::edge_weight_t>::type weight_map;

// matrix type
typedef boost::exterior_vertex_property<Graph, double> distance_property;
typedef distance_property::matrix_type distance_matrix;
typedef distance_property::matrix_map_type distance_matrix_map;

// write a distance-matrix to target filename
void output_matrix(char *filename, distance_matrix &matr, int N) {
  fprintf(stderr, "outputting shortest-path matrix to %s...\n", filename);
  FILE *output_f = fopen(filename, "w+");
  fprintf(output_f, "%d\n", N);
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      if(matr[i][j] == std::numeric_limits<double>::max()) {
        // do nothing - infinity
      } else {
        fprintf(output_f, "%.2f", matr[i][j]);
      }
      if (j < N - 1) {
        fputc(',', output_f);
      } else {
        fputc('\n', output_f);
      }
    }
  }
}

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "incorrect number of arguments\n");
    fprintf(stderr, "call as: ./main input_filename output_filename\n");
    return -1;
  }

  // read matrix
  FILE *input_f = fopen(argv[1], "r");

  int N;  // num nodes
  if (fscanf(input_f, "%d ", &N) < 1) {
    fprintf(stderr, "malformed input: couldn't match number N of vertices\n");
    return -1;
  }

  // create graph
  Graph G(N);
  weight_map weights = boost::get(boost::edge_weight, G);
  for (int i = 0; i < N; i++) {
    char inputValue[100];
    for (int j = 0; j < N; j++) {
      int numValues = fscanf(input_f, "%[^,\n]s", inputValue);
      if (numValues == 1) {
        auto e = boost::add_edge(i, j, G).first;
        weights[e] = strtof(inputValue, NULL);
      }
      fgetc(input_f);  // skip ',' or '\n'
    }
  }
  distance_matrix dist(N);
  distance_matrix_map map(dist, G);

  // algorithm call
  bool valid = floyd_warshall_all_pairs_shortest_paths(G, map,
                                                       boost::weight_map(weights));

  if (!valid) {
    fprintf(stderr, "invalid graph provided: negative cycle encountered!\n");
    return 1;
  }

  output_matrix(argv[2], dist, N);

  return 0;
}