# This is the Makefile used in the docker container for building things
# It should contain all the rules for building all the things.
# We get the following enironment variables passed from the local dockerfile:
#  - MAKE_BUILD_DIR

BUILD_DIR=$(MAKE_BUILD_DIR)

# Topmost rule must be to build the optimized C code

go-ref-shortest-path:
	cd shortest-path/go && go build -o ${BUILD_DIR}/go-ref-shortest-path .