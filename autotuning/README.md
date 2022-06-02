# Autotuning

## Run

```bash
python3 autotuning/autotune.py \
    -p $PWD \
    -l2 262144 \
    -algo mm \
    -n 256
```

## Plot

```bash
python measurements/perf-plots.py \
    --data measurements/data/fw_c-autotune*.csv  \
    --labels "algo" \
    --plot measurements/plots \
    --title "n256" \
    --output fw_c-autotune
```

## Analyze

```bash
python measurements/analyze.py \
    -p $PWD \
    -n 256
```
