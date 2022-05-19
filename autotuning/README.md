# Autotuning

## Run

```bash
python autotuning/fw-autotune.py \
    -p $PWD \
    -l1 131072 \
    -l2 1048576 \
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
