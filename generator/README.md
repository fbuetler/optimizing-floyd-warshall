# Graph generator

## Install dependencies

```bash
python3 -m pip install -r requirements.txt
```

## Run generator

```bash
python3 graph_generator.py \
    --nodes 30 \
    --edges 42 \
    --connected \
    --no-neg-cycle \
    --min-weight 0.0 \
    --max-weight 10.0 \
    --output graph.txt
```

**Note**: Connectivity for directed graphs is determined by weak connectivity.
A directed graph is weakly connected iff the underlying undirected graph (i.e. every directed edge
is turned into an undirected edge) is connected.

## Generate testcases
You can generate testcases with a dedicated script. Here is an example:
```bash
python3 generator/generate_tests.py -o testcases/n-from-100-to-1000-very-dense --min-n 100 --max-n 1000 -ddd --step 100
```
