# This is the Makefile used in the docker container for building things
# It should contain all the rules for building all the things.
# We get the following enironment variables passed from the local dockerfile:
#  - MAKE_BUILD_DIR

BUILD_DIR=$(MAKE_BUILD_DIR)
BUILD_NAME=$(MAKE_BUILD_NAME)
CFLAGS=$(MAKE_CFLAGS)

# Topmost rule must be to build the optimized C code

# fw - naive
fw-c-naive-gcc: shortest-path/c/*.c shortest-path/c/impl/naive.c shortest-path/c/impl/sp.h
	cd shortest-path/c; \
	gcc-11 $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) impl/naive.c main.c;

fw-c-naive-clang: shortest-path/c/*.c shortest-path/c/impl/naive.c shortest-path/c/impl/sp.h
	cd shortest-path/c; \
	clang-13 $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) impl/naive.c main.c;

# fw - unroll
fw-c-unroll-gcc: shortest-path/c/*.c shortest-path/c/impl/unroll.c shortest-path/c/impl/sp.h
	cd shortest-path/c; \
	gcc-11 $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) impl/unroll.c main.c;

fw-c-unroll-clang: shortest-path/c/*.c shortest-path/c/impl/unroll.c shortest-path/c/impl/sp.h
	cd shortest-path/c; \
	clang-13 $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) impl/unroll.c main.c;

# fw - cache blocking
fw-c-cache-blocking-gcc: shortest-path/c/*.c shortest-path/c/impl/cache_blocking.c shortest-path/c/impl/sp.h
	cd shortest-path/c; \
	gcc-11 $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) impl/cache_blocking.c main.c;

fw-c-cache-blocking-clang: shortest-path/c/*.c shortest-path/c/impl/cache_blocking.c shortest-path/c/impl/sp.h
	cd shortest-path/c; \
	clang-13 $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) impl/cache_blocking.c main.c;

# tc - naive
tc-c-naive-gcc: transitive-closure/c/*.c transitive-closure/c/impl/naive.c transitive-closure/c/impl/tc.h
	cd transitive-closure/c; \
	gcc-11 $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) impl/naive.c main.c;

tc-c-naive-clang: transitive-closure/c/*.c transitive-closure/c/impl/naive.c transitive-closure/c/impl/tc.h
	cd transitive-closure/c; \
	clang-13 $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) impl/naive.c main.c

# tc - unroll
tc-c-unroll-gcc: transitive-closure/c/*.c transitive-closure/c/impl/unroll.c transitive-closure/c/impl/tc.h
	cd transitive-closure/c; \
	gcc-11 $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) impl/unroll.c main.c;

tc-c-unroll-clang: transitive-closure/c/*.c transitive-closure/c/impl/unroll.c transitive-closure/c/impl/tc.h
	cd transitive-closure/c; \
	clang-13 $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) impl/unroll.c main.c

# max-min: naive
mm-c-naive-gcc: max-min/c/*.c max-min/c/impl/naive.c max-min/c/impl/mm.h
	cd max-min/c; \
	gcc-11 $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) impl/naive.c main.c;

mm-c-naive-clang: max-min/c/*.c max-min/c/impl/naive.c max-min/c/impl/mm.h
	cd max-min/c; \
	clang-13 $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) impl/naive.c main.c;

# max-min: unroll
mm-c-unroll-gcc: max-min/c/*.c max-min/c/impl/unroll.c max-min/c/impl/mm.h
	cd max-min/c; \
	gcc-11 $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) impl/unroll.c main.c;

mm-c-unroll-clang: max-min/c/*.c max-min/c/impl/unroll.c max-min/c/impl/mm.h
	cd max-min/c; \
	clang-13 $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) impl/unroll.c main.c;

# fw - go ref
fw-go-ref: shortest-path/go/*.go shortest-path/go/go.mod
	cd shortest-path/go && go build -o ${BUILD_DIR}/fw_go-ref .

# fw - boost ref
fw-boost-ref: shortest-path/boost/*
	cd shortest-path/boost && g++ $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) main.cpp

# fw - boost ref
tc-boost-ref: transitive-closure/boost/*
	cd transitive-closure/boost && g++ $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) main.cpp
