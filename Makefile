# This is the local Makefile
# It is used to build the docker container and execute other building steps in that

# Docker run template: docker run ${DOCKER_RUN_ARGS} make <target>

ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
BUILD_DIR_LOCAL:=${ROOT_DIR}/build
BUILD_DIR_DOCKER:=/build
BUILD_DIR_MOUNT:=--mount type=bind,source=${BUILD_DIR_LOCAL},target=${BUILD_DIR_DOCKER}
IMAGE_TAG:=asl-build
DOCKER_ENV_VARS:=-e MAKE_BUILD_DIR=${BUILD_DIR_DOCKER}
DOCKER_RUN_ARGS:=--rm ${BUILD_DIR_MOUNT} ${DOCKER_ENV_VARS} -t ${IMAGE_TAG}

# Topmost rule must be to build the optimized C code

build-c-naive-shortest-path: docker shortest-path/c/*.c shortest-path/c/impl/naive.c shortest-path/c/impl/sp.h
	docker run ${DOCKER_RUN_ARGS} make c-naive-shortest-path

build-c-naive-transitive-closure: docker transitive-closure/c/*.c transitive-closure/c/impl/naive.c transitive-closure/c/impl/tc.h
	docker run ${DOCKER_RUN_ARGS} make c-naive-transitive-closure

build-c-naive-max-min: docker max-min/c/*.c max-min/c/impl/naive.c max-min/c/impl/mm.h
	docker run ${DOCKER_RUN_ARGS} make c-naive-max-min

build-go-ref-shortest-path: docker shortest-path/go/*
	docker run ${DOCKER_RUN_ARGS} make go-ref-shortest-path

generate-graph: ${ROOT_DIR}/generator/graph_generator.py
	mkdir -p "$(shell dirname ${ROOT_DIR}/testcases/2/graph_n30_e42_min0_max10_connected.txt)"; \
	python3 ${ROOT_DIR}/generator/graph_generator.py \
    --nodes 30 \
    --edges 42 \
    --connected \
    --no-neg-cycle \
    --min-weight 0.0 \
    --max-weight 10.0 \
    --output ${ROOT_DIR}/testcases/2/graph_n30_e42_min0_max10_connected.txt

run-go-ref-shortest-path: ${BUILD_DIR_LOCAL}/go-ref-shortest-path ${ROOT_DIR}/testcases/2/graph_n30_e42_min0_max10_connected.txt
	${BUILD_DIR_LOCAL}/go-ref-shortest-path \
		-input-filename ${ROOT_DIR}/testcases/2/graph_n30_e42_min0_max10_connected.txt \
		-output-filename ${ROOT_DIR}/testcases/2/graph_n30_e42_min0_max10_connected.out.txt

run-python-ref-shortest-path: ${ROOT_DIR}/shortest-path/python/main.py ${ROOT_DIR}/testcases/2/graph_n30_e42_min0_max10_connected.txt
	python3 ${ROOT_DIR}/shortest-path/python/main.py \
		-i ${ROOT_DIR}/testcases/2/graph_n30_e42_min0_max10_connected.txt \
		-o ${ROOT_DIR}/testcases/2/graph_n30_e42_min0_max10_connected.out.txt
	
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