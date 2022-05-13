FROM debian:bookworm-slim

RUN apt-get -y update && apt-get -y upgrade && apt-get install -y --no-install-recommends \
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
