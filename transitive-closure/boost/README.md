# Transitive Closure with Boost
Benchmarking reference using the [Boost Graph Library](https://www.boost.org/).

## Build the binary
This depends on `g++` as well as `libboost-dev`.
```bash
g++ -O3 -o main.o main.cpp
```

## Run

```bash
./main.o example/simple.in example/simple.out
```
