# This is the Makefile used in the docker container for building things
# It should contain all the rules for building all the things.
# We get the following enironment variables passed from the local dockerfile:
#  - MAKE_BUILD_DIR

BUILD_DIR=$(MAKE_BUILD_DIR)
BUILD_NAME=$(MAKE_BUILD_NAME)
CFLAGS=$(MAKE_CFLAGS)

# Topmost rule must be to build the optimized C code

fw-c-unroll-gcc: shortest-path/c/*.c shortest-path/c/impl/unroll.c shortest-path/c/impl/sp.h
	cd shortest-path/c; \
	gcc-11 $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) impl/unroll.c main.c; 

fw-c-unroll-clang: shortest-path/c/*.c shortest-path/c/impl/unroll.c shortest-path/c/impl/sp.h
	cd shortest-path/c; \
	clang-13 $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) impl/unroll.c main.c;

fw-c-naive-gcc: shortest-path/c/*.c shortest-path/c/impl/naive.c shortest-path/c/impl/sp.h
	cd shortest-path/c; \
	gcc-11 $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) impl/naive.c main.c; 

fw-c-naive-clang: shortest-path/c/*.c shortest-path/c/impl/naive.c shortest-path/c/impl/sp.h
	cd shortest-path/c; \
	clang-13 $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) impl/naive.c main.c;

tc-c-naive-gcc: transitive-closure/c/*.c transitive-closure/c/impl/naive.c transitive-closure/c/impl/tc.h
	cd transitive-closure/c; \
	gcc-11 $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) impl/naive.c main.c; 

tc-c-naive-clang: transitive-closure/c/*.c transitive-closure/c/impl/naive.c transitive-closure/c/impl/tc.h
	cd transitive-closure/c; \
	clang-13 $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) impl/naive.c main.c 

mm-c-naive-gcc: max-min/c/*.c max-min/c/impl/naive.c max-min/c/impl/mm.h
	cd max-min/c; \
	gcc-11 $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) impl/naive.c main.c; 

mm-c-naive-clang: max-min/c/*.c max-min/c/impl/naive.c max-min/c/impl/mm.h
	cd max-min/c; \
	clang-13 $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) impl/naive.c main.c;

fw-go-ref: shortest-path/go/*.go shortest-path/go/go.mod
	cd shortest-path/go && go build -o ${BUILD_DIR}/fw_go-ref .

fw-boost-ref: shortest-path/boost/*
	cd shortest-path/boost && g++ $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) main.cpp

tc-boost-ref: transitive-closure/boost/*
	cd transitive-closure/boost && g++ $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) main.cpp
