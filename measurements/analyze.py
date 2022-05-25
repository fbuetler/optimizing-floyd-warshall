import os
import pandas as pd
import argparse

parser = argparse.ArgumentParser()
parser.add_argument(
    "-p", "--project-root", help="path of the project root", type=str, required=True
)
parser.add_argument(
    "-n", "--input-size", help="size of the input", type=int, required=True
)

pd.set_option("display.max_columns", None)
pd.set_option("display.max_rows", None)
pd.set_option("display.width", 1000)


def main(project_root, input_size):

    data_df = pd.DataFrame(
        columns=[
            "filename",
            "algorithm",
            "implementation",
            "compiler",
            "optimizations",
            "testsuite",
            "n",
            "runs",
            "cycles",
            "perf",
        ]
    )
    data_root = f"{project_root}/measurements/data"
    for fname in os.listdir(data_root):
        df = pd.read_csv(f"{data_root}/{fname}", sep=",")
        runs = df[f"{input_size}"][0]
        cycles = df[f"{input_size}"][1]
        perf = (2 * input_size**3) / cycles

        parts = fname.split("_")
        algorithm = parts[0]
        implementation = parts[1]
        compiler = parts[2]
        opts = " ".join(parts[3:-1])
        testsuite = parts[-1]

        data_point = pd.DataFrame(
            [
                [
                    fname,
                    algorithm,
                    implementation,
                    compiler,
                    opts,
                    testsuite,
                    input_size,
                    runs,
                    cycles,
                    perf,
                ]
            ],
            columns=[
                "filename",
                "algorithm",
                "implementation",
                "compiler",
                "optimizations",
                "testsuite",
                "n",
                "runs",
                "cycles",
                "perf",
            ],
        )

        data_df = pd.concat([data_df, data_point], ignore_index=True)

    data_df = data_df.sort_values(by=["perf"], ascending=False)
    print(data_df[["implementation", "cycles", "perf"]])


if __name__ == "__main__":
    args = parser.parse_args()
    main(args.project_root, args.input_size)
