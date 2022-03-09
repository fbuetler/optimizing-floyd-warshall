# This is the Makefile used in the docker container for building things
# It should contain all the rules for building all the things.
# We get the following enironment variables passed from the local dockerfile:
#  - MAKE_BUILD_DIR

BUILD_DIR=$(MAKE_BUILD_DIR)

# Topmost rule must be to build the optimized C code

c-naive-shortest-path: shortest-path/c-naive/*.c shortest-path/c-naive/*.h
	cd shortest-path/c-naive && gcc-11 -O3 -o $(BUILD_DIR)/c-naive-shortest-path sp.c main.c

c-naive-transitive-closure: transitive-closure/c-naive/*.c transitive-closure/c-naive/*.h
	cd transitive-closure/c-naive && gcc-11 -O3 -o $(BUILD_DIR)/c-naive-transitive-closure tc.c main.c

go-ref-shortest-path: shortest-path/go/*.go shortest-path/go/go.mod
	cd shortest-path/go && go build -o ${BUILD_DIR}/go-ref-shortest-path .