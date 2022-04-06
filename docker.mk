# This is the Makefile used in the docker container for building things
# It should contain all the rules for building all the things.
# We get the following enironment variables passed from the local dockerfile:
#  - MAKE_BUILD_DIR

BUILD_DIR=$(MAKE_BUILD_DIR)

# Topmost rule must be to build the optimized C code

c-naive-shortest-path: shortest-path/c/*.c shortest-path/c/impl/naive.c shortest-path/c/impl/sp.h
	cd shortest-path/c; \
	gcc-11 -O3 -o $(BUILD_DIR)/c-naive-shortest-path_gcc impl/naive.c main.c; \
	clang-13 -O3 -o $(BUILD_DIR)/c-naive-shortest-path_clang impl/naive.c main.c;

c-naive-transitive-closure: transitive-closure/c/*.c transitive-closure/c/impl/naive.c transitive-closure/c/impl/tc.h
	cd transitive-closure/c; \
	gcc-11 -O3 -o $(BUILD_DIR)/c-naive-transitive-closure_gcc impl/naive.c main.c; \
	clang-13 -O3 -o $(BUILD_DIR)/c-naive-transitive-closure_clang impl/naive.c main.c 

c-naive-max-min: max-min/c/*.c max-min/c/impl/naive.c max-min/c/impl/mm.h
	cd max-min/c; \
	gcc-11 -O3 -o $(BUILD_DIR)/c-naive-max-min_gcc impl/naive.c main.c; \
	clang-13 -O3 -o $(BUILD_DIR)/c-naive-max-min_clang impl/naive.c main.c;

go-ref-shortest-path: shortest-path/go/*.go shortest-path/go/go.mod
	cd shortest-path/go && go build -o ${BUILD_DIR}/go-ref-shortest-path .