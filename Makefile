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

TESTCASE_IN_PATH=${ROOT_DIR}/testcases/example/2/graph_n30_e42_min0_max10_connected.in.txt
TESTCASE_OUT_PATH=${ROOT_DIR}/testcases/example/2/graph_n30_e42_min0_max10_connected.out.txt
TESTCASE_DIR=${ROOT_DIR}/testcases/
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

compare:
	${ROOT_DIR}/comparator/compare.py -r ${ROOT_DIR}/testcases/

run-c-naive-transitive-closure: ${BUILD_DIR_LOCAL}/c-naive-transitive-closure ${TESTCASE_DIR}
	${ROOT_DIR}/comparator/runner.py \
		-b ${BUILD_DIR_LOCAL}/c-naive-transitive-closure \
		-d ${TESTCASE_DIR}

run-c-naive-max-min: ${BUILD_DIR_LOCAL}/c-naive-max-min ${TESTCASE_DIR}
	${ROOT_DIR}/comparator/runner.py \
		-b ${BUILD_DIR_LOCAL}/c-naive-max-min \
		-d ${TESTCASE_DIR}

run-c-naive-shortest-path: ${BUILD_DIR_LOCAL}/c-naive-shortest-path ${TESTCASE_DIR}
	${ROOT_DIR}/comparator/runner.py \
		-b ${BUILD_DIR_LOCAL}/c-naive-shortest-path \
		-d ${TESTCASE_DIR}

run-go-ref-shortest-path: ${BUILD_DIR_LOCAL}/go-ref-shortest-path ${TESTCASE_IN_PATH}
	${BUILD_DIR_LOCAL}/go-ref-shortest-path \
		-input-filename ${TESTCASE_IN_PATH} \
		-output-filename ${TESTCASE_OUT_PATH}

run-python-ref-shortest-path: ${ROOT_DIR}/shortest-path/python/main.py ${TESTCASE_IN_PATH}
	python3 ${ROOT_DIR}/shortest-path/python/main.py \
		-i ${TESTCASE_IN_PATH} \
		-o ${TESTCASE_OUT_PATH}

measure-c-naive-shortest-path: ${ROOT_DIR}/measurements/measure.py ${BUILD_DIR_LOCAL}/c-naive-shortest-path ${ROOT_DIR}/testcases/example ${ROOT_DIR}/measurements/data
	python3 ${ROOT_DIR}/measurements/measure.py \
		--binary ${BUILD_DIR_LOCAL}/c-naive-shortest-path \
		--testsuite ${ROOT_DIR}/testcases/example \
		--measurements ${ROOT_DIR}/measurements/data 

plot-c-naive-shortest-path: ${ROOT_DIR}/measurements/plot.py ${ROOT_DIR}/measurements/data/c-naive-shortest-path_example.csv ${ROOT_DIR}/measurements/plots
	python3 ${ROOT_DIR}/measurements/plot.py \
		--data ${ROOT_DIR}/measurements/data/c-naive-shortest-path_example.csv \
		--plot ${ROOT_DIR}/measurements/plots \
		--title "Example"

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
