FROM debian:bookworm-slim

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install --no-install-recommends --assume-yes \
    bash \
    ca-certificates \
    git \
    gcc-11 \
    clang-13 \
    libc6-dev \
    make \
    python3 \
    golang \
    g++ \
    libboost-dev

WORKDIR /home/work
RUN git clone https://bitbucket.org/icl/papi.git
WORKDIR /home/work/papi/src
RUN ./configure && make && make test && make install

RUN mkdir -p /home/work
WORKDIR /home/work

COPY docker.mk Makefile

COPY shortest-path ./shortest-path
COPY transitive-closure ./transitive-closure
COPY max-min ./max-min

RUN mkdir -p ./shortest-path/c/impl/generated
COPY autotuning/generated/fw ./shortest-path/c/impl

RUN mkdir -p ./transitive-closure/c/impl/generated
COPY autotuning/generated/tc ./transitive-closure/c/impl

RUN mkdir -p ./max-min/c/impl/generated
COPY autotuning/generated/mm ./max-min/c/impl
