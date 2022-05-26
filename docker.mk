SHELL=/bin/bash
# This is the Makefile used in the docker container for building things
# It should contain all the rules for building all the things.
# We get the following enironment variables passed from the local dockerfile:
#  - MAKE_BUILD_DIR

BUILD_DIR=$(MAKE_BUILD_DIR)
BUILD_NAME=$(MAKE_BUILD_NAME)
#static is needed for shared libs only present in the build container
CFLAGS=$(MAKE_CFLAGS) -I /usr/local/include -static
LDFLAGS:=/usr/local/lib/libpapi.a

# Topmost rule must be to build the optimized C code

# fw - vector
fw-c-vector-gcc: shortest-path/c/*.c shortest-path/c/impl/vector.c shortest-path/c/impl/sp.h
	cd shortest-path/c; \
	gcc-11 $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) impl/vector.c main.c $(LDFLAGS);

fw-c-vector-clang: shortest-path/c/*.c shortest-path/c/impl/vector.c shortest-path/c/impl/sp.h
	cd shortest-path/c; \
	clang-13 $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) impl/vector.c main.c $(LDFLAGS);

# fw - naive
fw-c-naive-gcc: shortest-path/c/*.c shortest-path/c/impl/naive.c shortest-path/c/impl/sp.h
	cd shortest-path/c; \
	gcc-11 $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) impl/naive.c main.c $(LDFLAGS);

fw-c-naive-clang: shortest-path/c/*.c shortest-path/c/impl/naive.c shortest-path/c/impl/sp.h
	cd shortest-path/c; \
	clang-13 $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) impl/naive.c main.c $(LDFLAGS);

# fw - unroll
fw-c-unroll-gcc: shortest-path/c/*.c shortest-path/c/impl/unroll.c shortest-path/c/impl/sp.h
	cd shortest-path/c; \
	gcc-11 $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) impl/unroll.c main.c $(LDFLAGS);

fw-c-unroll-clang: shortest-path/c/*.c shortest-path/c/impl/unroll.c shortest-path/c/impl/sp.h
	cd shortest-path/c; \
	clang-13 $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) impl/unroll.c main.c $(LDFLAGS);

# fw - cache blocking
fw-c-cache-blocking-gcc: shortest-path/c/*.c shortest-path/c/impl/cache_blocking.c shortest-path/c/impl/sp.h
	cd shortest-path/c; \
	gcc-11 $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) impl/cache_blocking.c main.c $(LDFLAGS);

fw-c-cache-blocking-clang: shortest-path/c/*.c shortest-path/c/impl/cache_blocking.c shortest-path/c/impl/sp.h
	cd shortest-path/c; \
	clang-13 $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) impl/cache_blocking.c main.c $(LDFLAGS);

# fw - autotuning
fw-c-autotune-gcc: shortest-path/c/*.c shortest-path/c/impl/fw_c-autotune*.c shortest-path/c/impl/sp.h
	cd shortest-path/c/impl; \
	for f in fw_c-autotune*.c; do \
		echo $$f; \
		gcc-11 $(CFLAGS) -o $(BUILD_DIR)/$${f%.*} "$$f" ../main.c $(LDFLAGS); \
	done 

fw-c-autotune-clang: shortest-path/c/*.c shortest-path/c/impl/fw_c-autotune*.c shortest-path/c/impl/sp.h
	cd shortest-path/c/impl; \
	for f in fw_c-autotune*.c; do \
		echo $$f; \
		clang-13 $(CFLAGS) -o $(BUILD_DIR)/$${f%.*} "$$f" ../main.c $(LDFLAGS); \
	done 

# tc - vector
tc-c-vector-gcc: transitive-closure/c/*.c transitive-closure/c/impl/vector.c transitive-closure/c/impl/tc.h
	cd transitive-closure/c; \
	gcc-11 $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) impl/vector.c main.c $(LDFLAGS);

tc-c-vector-clang: transitive-closure/c/*.c transitive-closure/c/impl/vector.c transitive-closure/c/impl/tc.h
	cd transitive-closure/c; \
	clang-13 $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) impl/vector.c main.c $(LDFLAGS)

# tc - naive
tc-c-naive-gcc: transitive-closure/c/*.c transitive-closure/c/impl/naive.c transitive-closure/c/impl/tc.h
	cd transitive-closure/c; \
	gcc-11 $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) impl/naive.c main.c $(LDFLAGS);

tc-c-naive-clang: transitive-closure/c/*.c transitive-closure/c/impl/naive.c transitive-closure/c/impl/tc.h
	cd transitive-closure/c; \
	clang-13 $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) impl/naive.c main.c $(LDFLAGS)

# tc - unroll
tc-c-unroll-gcc: transitive-closure/c/*.c transitive-closure/c/impl/unroll.c transitive-closure/c/impl/tc.h
	cd transitive-closure/c; \
	gcc-11 $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) impl/unroll.c main.c $(LDFLAGS);

tc-c-unroll-clang: transitive-closure/c/*.c transitive-closure/c/impl/unroll.c transitive-closure/c/impl/tc.h
	cd transitive-closure/c; \
	clang-13 $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) impl/unroll.c main.c $(LDFLAGS)

# tc - autotuning
tc-c-autotune-gcc: transitive-closure/c/*.c transitive-closure/c/impl/tc_c-autotune*.c transitive-closure/c/impl/tc.h
	cd transitive-closure/c/impl; \
	for f in tc_c-autotune*.c; do \
		echo $$f; \
		gcc-11 $(CFLAGS) -o $(BUILD_DIR)/$${f%.*} "$$f" ../main.c $(LDFLAGS); \
	done

tc-c-autotune-clang: transitive-closure/c/*.c transitive-closure/c/impl/tc_c-autotune*.c transitive-closure/c/impl/tc.h
	cd transitive-closure/c/impl; \
	for f in tc_c-autotune*.c; do \
		echo $$f; \
		clang-13 $(CFLAGS) -o $(BUILD_DIR)/$${f%.*} "$$f" ../main.c $(LDFLAGS); \
	done

# max-min: naive
mm-c-naive-gcc: max-min/c/*.c max-min/c/impl/naive.c max-min/c/impl/mm.h
	cd max-min/c; \
	gcc-11 $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) impl/naive.c main.c $(LDFLAGS);

mm-c-naive-clang: max-min/c/*.c max-min/c/impl/naive.c max-min/c/impl/mm.h
	cd max-min/c; \
	clang-13 $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) impl/naive.c main.c $(LDFLAGS);

# max-min: unroll
mm-c-unroll-gcc: max-min/c/*.c max-min/c/impl/unroll.c max-min/c/impl/mm.h
	cd max-min/c; \
	gcc-11 $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) impl/unroll.c main.c $(LDFLAGS);

mm-c-unroll-clang: max-min/c/*.c max-min/c/impl/unroll.c max-min/c/impl/mm.h
	cd max-min/c; \
	clang-13 $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) impl/unroll.c main.c $(LDFLAGS);

# max-min: vector
mm-c-vector-gcc: max-min/c/*.c max-min/c/impl/vector.c max-min/c/impl/mm.h
	cd max-min/c; \
	gcc-11 $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) impl/vector.c main.c $(LDFLAGS);

mm-c-vector-clang: max-min/c/*.c max-min/c/impl/vector.c max-min/c/impl/mm.h
	cd max-min/c; \
	clang-13 $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) impl/vector.c main.c $(LDFLAGS);

# max-min - autotuning
mm-c-autotune-gcc: max-min/c/*.c max-min/c/impl/mm_c-autotune*.c max-min/c/impl/mm.h
	cd max-min/c/impl; \
	for f in mm_c-autotune*.c; do \
		echo $$f; \
		gcc-11 $(CFLAGS) -o $(BUILD_DIR)/$${f%.*} "$$f" ../main.c $(LDFLAGS); \
	done

mm-c-autotune-clang: max-min/c/*.c max-min/c/impl/mm_c-autotune*.c max-min/c/impl/mm.h
	cd max-min/c/impl; \
	for f in mm_c-autotune*.c; do \
		echo $$f; \
		clang-13 $(CFLAGS) -o $(BUILD_DIR)/$${f%.*} "$$f" ../main.c $(LDFLAGS); \
	done

# fw - go ref
fw-go-ref: shortest-path/go/*.go shortest-path/go/go.mod
	cd shortest-path/go && go build -o ${BUILD_DIR}/fw_go-ref .

# fw - boost ref
fw-boost-ref: shortest-path/boost/*
	cd shortest-path/boost && g++ $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) main.cpp $(LDFLAGS)

# fw - boost ref
tc-boost-ref: transitive-closure/boost/*
	cd transitive-closure/boost && g++ $(CFLAGS) -o $(BUILD_DIR)/$(BUILD_NAME) main.cpp $(LDFLAGS)
