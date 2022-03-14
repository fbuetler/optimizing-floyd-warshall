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

**Note**: You cannot use both flags `--connect` and `--directed` at the same time.
