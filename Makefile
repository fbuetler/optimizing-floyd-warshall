# This is the local Makefile
# It is used to build the docker container and execute other building steps in that

# Docker run template: docker run ${DOCKER_RUN_ARGS} make <target>

ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
BUILD_DIR_LOCAL:=${ROOT_DIR}/build
BUILD_DIR_DOCKER:=/build
BUILD_NAME_DOCKER:=CHANGEME
CFLAGS_DOCKER:="-O3"
BUILD_DIR_MOUNT:=--mount type=bind,source=${BUILD_DIR_LOCAL},target=${BUILD_DIR_DOCKER}
IMAGE_TAG:=asl-build
DOCKER_ENV_VARS:=-e MAKE_BUILD_DIR=${BUILD_DIR_DOCKER} -e MAKE_BUILD_NAME=${BUILD_NAME_DOCKER} -e MAKE_CFLAGS="${CFLAGS_DOCKER}"
DOCKER_RUN_ARGS:=--rm ${BUILD_DIR_MOUNT} ${DOCKER_ENV_VARS} -t ${IMAGE_TAG}

# Topmost rule must be to build the optimized C code

# fw - vector
build-fw-c-vector-gcc: docker shortest-path/c/*.c shortest-path/c/impl/vector.c shortest-path/c/impl/sp.h
	docker run ${DOCKER_RUN_ARGS} make fw-c-vector-gcc

build-fw-c-vector-clang: docker shortest-path/c/*.c shortest-path/c/impl/vector.c shortest-path/c/impl/sp.h
	docker run ${DOCKER_RUN_ARGS} make fw-c-vector-clang

# mm - vector
build-mm-c-vector-gcc: docker max-min/c/*.c max-min/c/impl/vector.c max-min/c/impl/mm.h
	docker run ${DOCKER_RUN_ARGS} make mm-c-vector-gcc

build-mm-c-vector-clang: docker max-min/c/*.c max-min/c/impl/vector.c max-min/c/impl/mm.h
	docker run ${DOCKER_RUN_ARGS} make mm-c-vector-clang

# tc - vector
build-tc-c-vector-gcc: docker transitive-closure/c/*.c transitive-closure/c/impl/vector.c transitive-closure/c/impl/tc.h
	docker run ${DOCKER_RUN_ARGS} make tc-c-vector-gcc

build-tc-c-vector-clang: docker transitive-closure/c/*.c transitive-closure/c/impl/vector.c transitive-closure/c/impl/tc.h
	docker run ${DOCKER_RUN_ARGS} make tc-c-vector-clang

# TODO maybe pass implementations as an argument over env vars, this would remove all the repeating rules

# mm - unroll
build-mm-c-unroll-gcc: docker max-min/c/*.c max-min/c/impl/unroll.c max-min/c/impl/mm.h
	docker run ${DOCKER_RUN_ARGS} make mm-c-unroll-gcc

build-mm-c-unroll-clang: docker max-min/c/*.c max-min/c/impl/unroll.c max-min/c/impl/mm.h
	docker run ${DOCKER_RUN_ARGS} make mm-c-unroll-clang

# tc - unroll
build-tc-c-unroll-gcc: docker transitive-closure/c/*.c transitive-closure/c/impl/unroll.c transitive-closure/c/impl/tc.h
	docker run ${DOCKER_RUN_ARGS} make tc-c-unroll-gcc

build-tc-c-unroll-clang: docker transitive-closure/c/*.c transitive-closure/c/impl/unroll.c transitive-closure/c/impl/tc.h
	docker run ${DOCKER_RUN_ARGS} make tc-c-unroll-clang

# fw - unroll
build-fw-c-unroll-gcc: docker shortest-path/c/*.c shortest-path/c/impl/unroll.c shortest-path/c/impl/sp.h
	docker run ${DOCKER_RUN_ARGS} make fw-c-unroll-gcc

build-fw-c-unroll-clang: docker shortest-path/c/*.c shortest-path/c/impl/unroll.c shortest-path/c/impl/sp.h
	docker run ${DOCKER_RUN_ARGS} make fw-c-unroll-clang

# fw - naive
build-fw-c-naive-gcc: docker shortest-path/c/*.c shortest-path/c/impl/naive.c shortest-path/c/impl/sp.h
	docker run ${DOCKER_RUN_ARGS} make fw-c-naive-gcc

build-fw-c-naive-clang: docker shortest-path/c/*.c shortest-path/c/impl/naive.c shortest-path/c/impl/sp.h
	docker run ${DOCKER_RUN_ARGS} make fw-c-naive-clang

# fw - cache blocking
build-fw-c-cache-blocking-gcc: docker shortest-path/c/*.c shortest-path/c/impl/naive.c shortest-path/c/impl/sp.h
	docker run ${DOCKER_RUN_ARGS} make fw-c-cache-blocking-gcc

build-fw-c-cache-blocking-clang: docker shortest-path/c/*.c shortest-path/c/impl/naive.c shortest-path/c/impl/sp.h
	docker run ${DOCKER_RUN_ARGS} make fw-c-cache-blocking-clang

# fw autotune
build-fw-c-autotune-gcc: docker shortest-path/c/*.c autotuning/generated/fw/*.c shortest-path/c/impl/sp.h
	docker run ${DOCKER_RUN_ARGS} make fw-c-autotune-gcc

build-fw-c-autotune-clang: docker shortest-path/c/*.c autotuning/generated/fw/*.c shortest-path/c/impl/sp.h
	docker run ${DOCKER_RUN_ARGS} make fw-c-autotune-clang

# tc - naive
build-tc-c-naive-gcc: docker transitive-closure/c/*.c transitive-closure/c/impl/naive.c transitive-closure/c/impl/tc.h
	docker run ${DOCKER_RUN_ARGS} make tc-c-naive-gcc

build-tc-c-naive-clang: docker transitive-closure/c/*.c transitive-closure/c/impl/naive.c transitive-closure/c/impl/tc.h
	docker run ${DOCKER_RUN_ARGS} make tc-c-naive-clang

# tc autotune
build-tc-c-autotune-gcc: docker transitive-closure/c/*.c autotuning/generated/tc/*.c transitive-closure/c/impl/tc.h
	docker run ${DOCKER_RUN_ARGS} make tc-c-autotune-gcc

build-tc-c-autotune-clang: docker transitive-closure/c/*.c autotuning/generated/tc/*.c transitive-closure/c/impl/tc.h
	docker run ${DOCKER_RUN_ARGS} make tc-c-autotune-clang

# max-min: naive
build-mm-c-naive-gcc: docker max-min/c/*.c max-min/c/impl/naive.c max-min/c/impl/mm.h
	docker run ${DOCKER_RUN_ARGS} make mm-c-naive-gcc

build-mm-c-naive-clang: docker max-min/c/*.c max-min/c/impl/naive.c max-min/c/impl/mm.h
	docker run ${DOCKER_RUN_ARGS} make mm-c-naive-clang

# max-min autotune
build-mm-c-autotune-gcc: docker max-min/c/*.c autotuning/generated/mm/*.c max-min/c/impl/mm.h
	docker run ${DOCKER_RUN_ARGS} make mm-c-autotune-gcc

build-mm-c-autotune-clang: docker max-min/c/*.c autotuning/generated/mm/*.c max-min/c/impl/mm.h
	docker run ${DOCKER_RUN_ARGS} make mm-c-autotune-clang

# fw - go ref
build-fw-go-ref: docker shortest-path/go/*
	docker run ${DOCKER_RUN_ARGS} make fw-go-ref

# fw - boost ref
build-fw-boost-ref: docker shortest-path/boost/*
	docker run ${DOCKER_RUN_ARGS} make fw-boost-ref

# tc - boost ref
build-tc-boost-ref: docker transitive-closure/boost/*
	docker run ${DOCKER_RUN_ARGS} make tc-boost-ref

.PHONY: docker
docker: Dockerfile .dockerignore
	docker build -t ${IMAGE_TAG} .

.PHONY: clean
clean:
	rm -rf ${BUILD_DIR_LOCAL}/*

# List recipe taken from https://stackoverflow.com/a/26339924
.PHONY: list
list:
	@LC_ALL=C $(MAKE) -pRrq -f $(lastword $(MAKEFILE_LIST)) : 2>/dev/null | awk -v RS= -F: '/^# File/,/^# Finished Make data base/ {if ($$1 !~ "^[#.]") {print $$1}}' | sort | egrep -v -e '^[^[:alnum:]]' -e '^$@$$'
