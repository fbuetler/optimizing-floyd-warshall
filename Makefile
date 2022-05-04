# This is the local Makefile
# It is used to build the docker container and execute other building steps in that

# Docker run template: docker run ${DOCKER_RUN_ARGS} make <target>

ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
BUILD_DIR_LOCAL:=${ROOT_DIR}/build
BUILD_DIR_DOCKER:=/build
CFLAGS_DOCKER:="-O3"
BUILD_DIR_MOUNT:=--mount type=bind,source=${BUILD_DIR_LOCAL},target=${BUILD_DIR_DOCKER}
IMAGE_TAG:=asl-build
DOCKER_ENV_VARS:=-e MAKE_BUILD_DIR=${BUILD_DIR_DOCKER} -e MAKE_CFLAGS="${CFLAGS_DOCKER}"
DOCKER_RUN_ARGS:=--rm ${BUILD_DIR_MOUNT} ${DOCKER_ENV_VARS} -t ${IMAGE_TAG}

# Topmost rule must be to build the optimized C code

build-fw-c-naive: docker shortest-path/c/*.c shortest-path/c/impl/naive.c shortest-path/c/impl/sp.h
	docker run ${DOCKER_RUN_ARGS} make fw-c-naive

build-tc-c-naive: docker transitive-closure/c/*.c transitive-closure/c/impl/naive.c transitive-closure/c/impl/tc.h
	docker run ${DOCKER_RUN_ARGS} make tc-c-naive

build-mm-c-naive: docker max-min/c/*.c max-min/c/impl/naive.c max-min/c/impl/mm.h
	docker run ${DOCKER_RUN_ARGS} make mm-c-naive

build-fw-go-ref: docker shortest-path/go/*
	docker run ${DOCKER_RUN_ARGS} make fw-go-ref

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
