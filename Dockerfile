FROM debian:bookworm-slim

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install --no-install-recommends --assume-yes \
    bash \
    gcc-11 \
    clang-13 \
    libc6-dev \
    make \
    python3 \
    golang \
    g++ \
    libboost-dev

RUN mkdir -p /home/work
WORKDIR /home/work

COPY docker.mk Makefile

COPY shortest-path ./shortest-path
COPY transitive-closure ./transitive-closure
COPY max-min ./max-min

RUN mkdir -p ./shortest-path/c/impl/generated
COPY autotuning/generated/shortest-path ./shortest-path/c/impl
