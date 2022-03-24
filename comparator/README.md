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

# Directory Wide Compare
To compare everything within a given directory and all its subdirectories use `-r` or `--recursive`:
```bash
./compare.py -r path/to/root/directory
```

The only extra argument that is compatible with recursive is `-p` / `--precision=`. It will be used for everything except `.tc` files (so for `.fw` and `.mm`).

The script will compare each pair of out- and reference-files (see wiki: Input and Output Formats), and print the result.
At the end, it will also print the total number of passes and tests.

Exit codes are similar to comparing 2 files: 0 if everything passes, 1 if at least one test fails, 2 in case of an error.

# Binary Runner
Run a binary against all input-files within a specified directory and its subdirectories.

## Run Runner
The runner takes 4 arguments:
1. `-b` / `--binary`: the binary to use on all found input-files.
2. `-a` / `--algorithm`: the algorithm-part to use in output file-endings (see wiki).
   For instance, `fw` for floyd-warshall.
3. `-o` / `--output`: output-type to use in file-ending (`ref` for reference files, `out` for out-files).
4. `-d` / `--directory`: root directory to traverse / find testcases in.
