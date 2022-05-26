# Team 7

## The almighty bash script

```bash
$ ./team7.sh
Usage: ./team7.sh
Commands:
  build <ALGORITHM_LIST> <IMPLEMENTATION_LIST> <COMPILER_LIST> <OPTIMIZATIONS_LIST>
  generate-test-in <MIN_N> <MAX_N> <STEP_N>
  generate-test-out <ALGORITHM> <REF_IMPL> <INPUT_CATEGORY>
  validate <ALGORITHM_LIST> <IMPLEMENTATION_LIST> <COMPILER_LIST> <OPTIMIZATIONS_LIST> (<TESTCASES>)
  measure <ALGORITHM_LIST> <IMPLEMENTATION_LIST> <COMPILER_LIST> <OPTIMIZATIONS_LIST> (<INPUT_CATEGORY>)
  plot <ALGORITHM_LIST> <IMPLEMENTATION_LIST> <COMPILER_LIST> <OPTIMIZATIONS_LIST> (<INPUT_CATEGORY>) <PLOT_TITLE> (<PLOT_LABELS_LIST>)
  clean
[...]
```

## Building

For reproducible builds we use [docker](https://www.docker.com/get-started) in combination with [make](https://www.gnu.org/software/make).
We have two makefiles: one in the docker container [`docker.mk`](docker.mk) and [`Makefile`](Makefile) locally.
The file `docker.mk` contains the building recipies that are executed inside the docker container defined in
the [`Dockerfile`](Dockerfile). The local [`Makefile`](Makefile) executes make commands in the docker container
and the results end up in the `build/` directory.

To build the optimized code run

```bash
$ make
```

To list available targets, run

```bash
$ make list
```

Alternatively, multiple targets can be built using the almighty bashscript. For example, to compile the naive and the unrolled C implementations of the max-min algorithm using clang and the -O3 and -fno-tree-vectorize compiler flags, we'd do the following:

```bash
$ ./team7.sh build mm c-naive,c-unroll clang '-O3 -fno-tree-vectorize'
```

Note that the corresponding Makefile recipes need to have been added, as described bellow.

### How to add Stuff I want to build?

1. Make sure the toolchain required to build the program is present in the container or add it in the [`Dockerfile`](Dockerfile).
2. Add a recipe in `docker.mk` that is executed in the docker container. Make sure that the output goes
   into the build directory (use the `${BUILD_DIR}` variable) and that the recipe for the optimized code
   stays at the top. Please also ensure that the recipe follows the naming scheme {algorithm}-{implementation}-{compiler}. For example, a recipe to build the vectorized C implementation for the shortest path algorithm compiled with gcc might look like this:
   ```make
   fw-c-vector-gcc: shortest-path/c/*.c shortest-path/c/impl/vector.c shortest-path/c/impl/sp.h
	cd shortest-path/c; \
	gcc-11 $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) impl/vector.c main.c $(LDFLAGS);
   ```
3. Add a recipe in the `Makefile` which runs the appropriate `make` target in the container. Use
   `docker run ${DOCKER_RUN_ARGS} make <target in docker.mk>` as a template. Again, please follow the aforementioned naming scheme, now prefixed with 'build-'. For example, to build the vectorized C implementation for the shortest path algorithm inside the docker container, add the following rule:
   ```make
   build-fw-c-vector-gcc: docker shortest-path/c/*.c shortest-path/c/impl/vector.c shortest-path/c/impl/sp.h
	docker run ${DOCKER_RUN_ARGS} make fw-c-vector-gcc
   ```

## Validating an implementation

![ASL Overview](./assets/ASL_Overview.png)

The `validate` command of the almighty bashscript follows the same syntax as the `build` command. For example, to validate the naive C implementation of the max-min algorithm using clang and the -O3 and -fno-tree-vectorize compiler flags, we'd do the following:

```bash
$ ./team7.sh validate mm c-naive clang '-O3 -fno-tree-vectorize'
```

The output will look something like this if all went well:

```bash
Validating mm/c-naive

Directory:               testcases/test-inputs//n4:
====================================================
- graph_n4_e4_min1_max10_directed.mm:            PASS

Directory:              testcases/test-inputs//n16:
====================================================
- graph_n16_e480_min1_max2_dense2_connected.mm:  PASS

Directory:               testcases/test-inputs//n8:
====================================================
- graph_n8_e136_min1_max2_dense2_connected.mm:   PASS

Directory:              testcases/test-inputs//n32:
====================================================
- graph_n32_e1536_min1_max2_dense2_connected.mm: PASS

Directory:              testcases/test-inputs//n30:
====================================================
- graph_n30_e42_min0_max10_connected.mm:         PASS

5 of 5 tests (100%) passed!
```

Note that the command won't necessarily work for multiple implementations. Furthermore, please ensure you've set up and activated a virtual environment for Python (or alternatively, installed all the packages as listed in `requirements.txt`).

## Running a benchmark

The `measure` command of the almighty bashscript again follows the same syntax. For example, to run the naive and unrolled C implementations of the max-min algorithm using clang and the -O3 and -fno-tree-vectorize compiler flags on the benchmark inputs, we'd do the following:

```bash
$ ./team7.sh measure mm c-naive,c-unroll clang '-O3 -fno-tree-vectorize' bench-inputs
```

For each testcase (i.e. for each value of `n`), the command will output the number of runs, the average number of cycles per run and the numbers of L1, L2 and L3 cache misses. These values will also be stored in a csv file in the directory described by the `MEASUREMENTS_DIR` env variable, which is set to `${ROOT_DIR}/measurements/data` by default.

## Generating plots

In order to compare the performance of the implementations, plots must be generated. Two types of plots are currently supported: Performance plots, and roofline plots.

### Performance plots

Performance plots can be generated using the almighty bashscript for implementations that were compiled using identical compilers and compiler flags. For example, to plot the obtained measurements for the naive and unrolled C implementations of the max-min algorithm compiled using clang and the -O3 and -fno-tree-vectorize compiler flags on the benchmark inputs, we'd do the following:

```bash
$ ./team7.sh plot mm c-naive,c-unroll clang '-O3 -fno-tree-vectorize' bench-inputs 'Max-Min Optimizations' impl
```

where 'Max-Min Optimizations' indicates the title to use for the generated plot and `impl` indicates that the implementation names (e.g. 'c-naive') should be used to label the plotted lines.

Alternatively, the python script can be called directly to plot more diverse data. For example, if the naive C implementation were compiled using only the `-O3` flag, we could call the script from the `measurements` directory as follows:

```bash
$ python3 perf-plots.py --data data/mm_c-naive_clang_-O3_bench-inputs.csv data/mm_c-unroll_clang_-O3_-fno-tree-vectorize_bench-inputs.csv --plot plots --title 'Max-Min Optimizations' --labels impl --output 'mm-optimizations' -pi 2.0 -vpi 8.0
```

The result will be stored in the `plots` directory.

### Roofline plots

Roofline plots can be generated by hand using the `roof-plots.py` script. For example, to plot the obtained measurements for the naive and unrolled C implementations of the max-min algorithm compiled using clang and the -O3 and -fno-tree-vectorize compiler flags on the benchmark inputs, we'd do the following:

```bash
$ python3 roof-plots.py --data data/mm_c-naive_clang_-O3_bench-inputs.csv data/mm_c-unroll_clang_-O3_-fno-tree-vectorize_bench-inputs.csv --plot plots --title 'Max-Min Optimizations' --labels opts --output 'mm-optimizations-roofline' -pi 2.0 -vpi 8.0 -b 32
```

where `pi` and `vpi` correspond to non- and vectorized peak performance, respectively, and `b` corresponds to peak memory bandwidth, as measured using the STREAM benchmark.

## Generate a graph AKA testcase input

```bash
$ make generate-graph
```

## Run a reference implementation AKA testcase output

```bash
$ make run-go-ref-shortest-path
$ make run-python-ref-shortest-path
```
