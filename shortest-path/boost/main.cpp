#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/property_maps/constant_property_map.hpp>
#include <boost/graph/floyd_warshall_shortest.hpp>
#include <boost/graph/directed_graph.hpp>
#include <boost/graph/exterior_property.hpp>
#include <papi.h>

#define NUM_RUNS 16
#define CYCLES_REQUIRED 1e8
#define FREQUENCY 2.3e9
#define CALIBRATE

#define ERROR_RETURN(retval)                                                     \
  {                                                                              \
    fprintf(stderr, "PAPI error %d %s:line %d: \n", retval, __FILE__, __LINE__); \
    exit(retval);                                                                \
  }

// The number of PAPI events we measure
#define NUM_EVENT 4

// The PAPI events we subscribe to for the measurement
// Only add events supported on your machine. Check with the papi_avail tool in the PAPI utils
// IMPORTANT: DO NOT CHANGE ORDER! Otherwise the data readout in the measurement script breaks
static int event_codes[NUM_EVENT] = {
    PAPI_TOT_CYC, // Total cycles executed
    PAPI_L3_TCM,  // L3 cache misses
    PAPI_L2_TCM,  // L2 data cache misses
    PAPI_L1_DCM,  // L1 data cache misses
};

// types
// graph type
typedef boost::property<boost::edge_weight_t, double> edge_weight_property;
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS,
                              boost::no_property, edge_weight_property>
    Graph;
typedef boost::property_map<Graph, boost::edge_weight_t>::type weight_map;

// matrix type
typedef boost::exterior_vertex_property<Graph, double> distance_property;
typedef distance_property::matrix_type distance_matrix;
typedef distance_property::matrix_map_type distance_matrix_map;

// write a distance-matrix to target filename
void output_matrix(char *filename, distance_matrix &matr, int N)
{
  fprintf(stderr, "outputting shortest-path matrix to %s...\n", filename);
  FILE *output_f = fopen(filename, "w+");
  fprintf(output_f, "%d\n", N);
  for (int i = 0; i < N; i++)
  {
    for (int j = 0; j < N; j++)
    {
      if (matr[i][j] == std::numeric_limits<double>::max())
      {
        // do nothing - infinity
      }
      else
      {
        fprintf(output_f, "%.2f", matr[i][j]);
      }
      if (j < N - 1)
      {
        fputc(',', output_f);
      }
      else
      {
        fputc('\n', output_f);
      }
    }
  }
}

/*
 * Timing function based on the TimeStep Counter of the CPU.
 *
 * We don't particularly care about the input, so we just
 * run FW on the same matrix C for each run even though it
 * modifies it in-place.
 *
 * The function returns the average number of cycles per run.
 */
int measure(Graph G, distance_matrix_map map, weight_map weights,
            int WarmupEventSet, int MeasurementEventSet, long long *measured_values)
{
  int i, retval;
  int num_runs = NUM_RUNS;

  // Compute how many runs we need to do and warm up cache
  long long warmup_cycles[1];
  /* Start counting */
  long long cycles = 0;
  while (num_runs < (1 << 14))
  {
    if ((retval = PAPI_start(WarmupEventSet)) != PAPI_OK)
    {
      ERROR_RETURN(retval);
    }

    for (i = 0; i < num_runs; ++i)
    {

      floyd_warshall_all_pairs_shortest_paths(G, map, boost::weight_map(weights));
    }

    /* Stop counting, this reads from the counter as well as stop it. */
    if ((retval = PAPI_stop(WarmupEventSet, warmup_cycles)) != PAPI_OK)
    {
      ERROR_RETURN(retval);
    }

    cycles += warmup_cycles[0];

    if (cycles >= CYCLES_REQUIRED)
    {
      break;
    }

    num_runs *= 2;
    if ((retval = PAPI_reset(WarmupEventSet)) != PAPI_OK)
    {
      ERROR_RETURN(retval);
    }
  }

  fprintf(stderr, "#runs: %d\n", num_runs);
  /*
   * Alternatives to the current approach:
   * - Measure #cycles for each run separately
   *  --> Suffers from timing bias and requires cache warmup
   *  --> Allows for more fine-grained statistics, e.g. median, variance, etc.
   */
  /* Start counting */
  if ((retval = PAPI_start(MeasurementEventSet)) != PAPI_OK)
  {
    ERROR_RETURN(retval);
  }

  for (i = 0; i < num_runs; ++i)
  {
    floyd_warshall_all_pairs_shortest_paths(G, map, boost::weight_map(weights));
  }

  /* Stop counting, this reads from the counter as well as stop it. */
  if ((retval = PAPI_stop(MeasurementEventSet, measured_values)) != PAPI_OK)
  {
    ERROR_RETURN(retval);
  }

  return num_runs;
}

int main(int argc, char **argv)
{
  if (argc != 3)
  {
    fprintf(stderr, "incorrect number of arguments\n");
    fprintf(stderr, "call as: ./main input_filename output_filename\n");
    return -1;
  }

  // setup PAPI
  int retval;
  int WarmupEventSet = PAPI_NULL;
  int MeasurementEventSet = PAPI_NULL;
  char errstring[PAPI_MAX_STR_LEN];
  long long measured_values[NUM_EVENT];

  if ((retval = PAPI_library_init(PAPI_VER_CURRENT)) != PAPI_VER_CURRENT)
  {
    fprintf(stderr, "Error: %s\n", errstring);
    return 1;
  }

  /* Creating event set   */
  if ((retval = PAPI_create_eventset(&MeasurementEventSet)) != PAPI_OK)
  {
    ERROR_RETURN(retval);
  }
  if ((retval = PAPI_create_eventset(&WarmupEventSet)) != PAPI_OK)
  {
    ERROR_RETURN(retval);
  }

  // add events to event sets
  if ((retval = PAPI_add_event(WarmupEventSet, PAPI_TOT_CYC)) != PAPI_OK)
  {
    ERROR_RETURN(retval);
  }

  if ((retval = PAPI_add_events(MeasurementEventSet, event_codes, NUM_EVENT)) != PAPI_OK)
  {
    ERROR_RETURN(retval);
  }

  // read matrix
  FILE *input_f = fopen(argv[1], "r");

  int N; // num nodes
  if (fscanf(input_f, "%d ", &N) < 1)
  {
    fprintf(stderr, "malformed input: couldn't match number N of vertices\n");
    return -1;
  }

  // create graph
  Graph G(N);
  weight_map weights = boost::get(boost::edge_weight, G);
  for (int i = 0; i < N; i++)
  {
    char inputValue[100];
    for (int j = 0; j < N; j++)
    {
      int numValues = fscanf(input_f, "%[^,\n]s", inputValue);
      if (numValues == 1)
      {
        auto e = boost::add_edge(i, j, G).first;
        weights[e] = strtof(inputValue, NULL);
      }
      fgetc(input_f); // skip ',' or '\n'
    }
  }
  distance_matrix dist(N);
  distance_matrix_map map(dist, G);

  // algorithm call
  bool valid = floyd_warshall_all_pairs_shortest_paths(G, map,
                                                       boost::weight_map(weights));

  if (!valid)
  {
    fprintf(stderr, "invalid graph provided: negative cycle encountered!\n");
    return 1;
  }

  output_matrix(argv[2], dist, N);

  // run measurements
  fprintf(stderr, "measuring shortest paths for n=%d\n", N);
  int num_runs = measure(G, map, weights, WarmupEventSet, MeasurementEventSet, measured_values);

  // output measurements
  printf("%d", num_runs);
  for (int i = 0; i < NUM_EVENT; i++)
  {
    printf("\n%lld", measured_values[i] / num_runs);
  }

  /* Free all memory and data structures, EventSet must be empty. */
  if ((retval = PAPI_cleanup_eventset(WarmupEventSet)) != PAPI_OK)
  {
    ERROR_RETURN(retval);
  }
  if ((retval = PAPI_cleanup_eventset(MeasurementEventSet)) != PAPI_OK)
  {
    ERROR_RETURN(retval);
  }

  if ((retval = PAPI_destroy_eventset(&WarmupEventSet)) != PAPI_OK)
  {
    ERROR_RETURN(retval);
  }
  if ((retval = PAPI_destroy_eventset(&MeasurementEventSet)) != PAPI_OK)
  {
    ERROR_RETURN(retval);
  }

  /* free the resources used by PAPI */
  PAPI_shutdown();

  return EXIT_SUCCESS;
}
