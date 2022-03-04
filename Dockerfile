FROM debian:bookworm-slim

RUN apt-get update && \
    apt-get upgrade

RUN apt-get install -y --no-install-recommends \
    bash \
    gcc-11 \
    libc6-dev \
    make \
    python3 \
    golang

RUN mkdir -p /home/work
WORKDIR /home/work

COPY docker.mk Makefile

COPY shortest-path ./shortest-path
COPY transitive-closure ./transitive-closure
