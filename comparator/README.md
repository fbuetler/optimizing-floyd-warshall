# Graph Comparator

## Install dependencies

```bash
python3 -m pip install -r requirements.txt
```

## Run Comparator

The files to read matrices from and compare are regular arguments.
```bash
./compare.py matrix1.txt matrix2.txt
```

The script will terminate with exit code 0 for equal matrices, 1 for unequal matrices, and 2 in case of an error.

## Extra Arguments
- To set the precision to which the matrices are compared, use `-p` or `--precision=`, default is `0.01`. Use `-p inf` or `--precision=inf` to compare function graphs.
- To generate an image of the graph, with differences in red, use `-i` or `--img=` to specify the target path (without '.png' file ending).
- Use `-s` or `--silent` to disable messages.

**Note**: These extra arguments all have to come before the two matrices to compare!
