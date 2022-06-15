#!/bin/bash

# Safer bash scripts
# @see https://explainshell.com/explain?cmd=set+-euCo+pipefail
# @see https://vaneyckt.io/posts/safer_bash_scripts_with_set_euxo_pipefail/
set -euCo pipefail

# Configuration variables
ROOT_DIR=$(dirname "$0")
BUILD_DIR="${ROOT_DIR}/build"
INPUT_CATEGORY_DIR="${ROOT_DIR}/testcases"
TESTCASE_DIR="${INPUT_CATEGORY_DIR}/test-inputs"
BENCHMARK_DIR="${INPUT_CATEGORY_DIR}/bench-inputs"
MEASUREMENTS_DIR="${ROOT_DIR}/measurements/data"
PLOTS_DIR="${ROOT_DIR}/measurements/plots"
GENERIC_DIR="${ROOT_DIR}/generic"

function printUsage() {
    echo "Usage: $0"
    echo "Commands:"
    echo "  build <ALGORITHM_LIST> <IMPLEMENTATION_LIST> <COMPILER_LIST> <OPTIMIZATIONS_LIST>"
    echo "  generate-test-in <MIN_N> <MAX_N> <STEP_N>"
    echo "  generate-test-out <ALGORITHM> <REF_IMPL> <INPUT_CATEGORY>"
    echo "  validate <ALGORITHM_LIST> <IMPLEMENTATION_LIST> <COMPILER_LIST> <OPTIMIZATIONS_LIST> (<TESTCASES>)"
    echo "  measure <ALGORITHM_LIST> <IMPLEMENTATION_LIST> <COMPILER_LIST> <OPTIMIZATIONS_LIST> (<INPUT_CATEGORY>)"
    echo "  plot <ALGORITHM_LIST> <IMPLEMENTATION_LIST> <COMPILER_LIST> <OPTIMIZATIONS_LIST> (<INPUT_CATEGORY>) <PLOT_TITLE> (<PLOT_LABELS_LIST>)"
    echo "  clean"
    echo "Algorithms:"
    echo "  fw (floyd-wahrshal)"
    echo "  tc (transitive closure)"
    echo "  mm (max-min)"
    echo "Implementations:"
    echo "  c-naive"
    echo "  c-unroll"
    echo "  c-vector"
    echo "  boost"
    echo "Reference implementations:"
    echo "  go-ref"
    echo "Compilers:"
    echo "  gcc"
    echo "  clang"
    echo "Optimization examples:"
    echo "  -O0"
    echo "  -O3 -fno-tree-vectorize"
    echo "  -O3 -ffast-math -march=native -mfma"
    echo "Testcases:"
    echo "  n4,n8,n16,n30,n32"
    echo "  n64,n128,n256,n512,n1024,n2048,n4096,n8192"
    echo "Input categories:"
    echo "  $(ls -m $INPUT_CATEGORY_DIR | sed 's/,/\n /g')"
    echo "Plot labels"
    echo "  algo"
    echo "  impl"
    echo "  comp"
    echo "  opts"
    echo "  testsuite"
    echo "NOTE:"
    echo "  input lists have to be comma separated"
    exit 1
}

function optimizations_format() {
    OPTIMIZATIONS_RAW="$1"
    echo $OPTIMIZATIONS_RAW | tr " " "_"
}

function build() {
    ALGORITHM="$1"
    IMPLEMENTATION="$2"
    COMPILER="$3"
    OPTIMIZATIONS_RAW="$4"
    OPTIMIZATIONS=$(optimizations_format "$OPTIMIZATIONS_RAW")

    make "build-${ALGORITHM}-${IMPLEMENTATION}-${COMPILER}" \
        -e BUILD_NAME_DOCKER="${ALGORITHM}_${IMPLEMENTATION}_${COMPILER}_${OPTIMIZATIONS}" \
        -e CFLAGS_DOCKER="$OPTIMIZATIONS_RAW"
}

function generate-test-in() {
    MIN_N="$1"
    MAX_N="$2"
    STEP_N="$3"
    python3 "${ROOT_DIR}/generator/generate_tests.py" \
        -o "${TESTCASE_DIR}/n-from-$MIN_N-to-$MAX_N-very-dense" \
        --min-n "$MIN_N" \
        --max-n "$MAX_N" \
        --step "$STEP_N" \
        -ddd
}

function generate-test-out() {
    ALGORITHM="$1"
    REF_IMPL="$2"
    INPUT_CATEGORY="$3"
    TESTCASES=$(find ${INPUT_CATEGORY_DIR}/${INPUT_CATEGORY} -name "*.in.txt")
    # TODO generate mm.ref and tc.ref too
    for TESTCASE_IN in $TESTCASES; do
        "${BUILD_DIR}/${ALGORITHM}_${REF_IMPL}" \
            -input-filename "${TESTCASE_IN}" \
            -output-filename "${TESTCASE_IN%%.in.txt}.fw.ref.txt"
    done
}

function validate() {
    ALGORITHM="$1"
    IMPLEMENTATION="$2"
    COMPILER="$3"
    OPTIMIZATIONS_RAW="$4"
    OPTIMIZATIONS=$(optimizations_format "$OPTIMIZATIONS_RAW")
    TESTCASES="$5"

    rm -f $TESTCASES/**/*.out.txt
    python3 "${ROOT_DIR}/comparator/runner.py" \
        -b "${BUILD_DIR}/${ALGORITHM}_${IMPLEMENTATION}_${COMPILER}_${OPTIMIZATIONS}" \
        -d "${TESTCASES}" \
        -a "${ALGORITHM}" \
        -o "out"

    python3 "${ROOT_DIR}/comparator/compare.py" \
        --recursive \
        --silent \
        "${TESTCASES}"
}

function measure() {
    ALGORITHM="$1"
    IMPLEMENTATION="$2"
    COMPILER="$3"
    OPTIMIZATIONS_RAW="$4"
    OPTIMIZATIONS=$(optimizations_format "$OPTIMIZATIONS_RAW")
    INPUT_CATEGORY="$5"
    TESTCASES="$6"
    python3 "${ROOT_DIR}/measurements/measure.py" \
        --binary "${BUILD_DIR}/${ALGORITHM}_${IMPLEMENTATION}_${COMPILER}_$OPTIMIZATIONS" \
        --testsuite "${INPUT_CATEGORY_DIR}/${INPUT_CATEGORY}" \
        --testcases "${TESTCASES}" \
        --output "${MEASUREMENTS_DIR}/${ALGORITHM}_${IMPLEMENTATION}_${COMPILER}_${OPTIMIZATIONS}_${INPUT_CATEGORY}" \
        --incremental # remove this to overwrite existing results
}

function plot() {
    ALGORITHM="$1"
    IMPLEMENTATION="$2"
    COMPILER="$3"
    OPTIMIZATIONS_RAW="$4"
    OPTIMIZATIONS=$(optimizations_format "$OPTIMIZATIONS_RAW")
    INPUT_CATEGORY="$5"
    PLOT_TITLE="$6"
    PLOT_LABELS="$7"
    python3 "${ROOT_DIR}/measurements/perf-plots.py" \
        --data "${MEASUREMENTS_DIR}/${ALGORITHM}_${IMPLEMENTATION}_${COMPILER}_${OPTIMIZATIONS}_${INPUT_CATEGORY}.csv" \
        --plot "${PLOTS_DIR}" \
        --title "$PLOT_TITLE" \
        --labels "${PLOT_LABELS[@]}" \
        --output "${ALGORITHM}_${IMPLEMENTATION}_${COMPILER}_${OPTIMIZATIONS}_${INPUT_CATEGORY}"
}

function plot-final() {
    # sp
    python3 measurements/perf-plots.py \
        --data $(
            cat <<EOM
measurements/data-mhaessig/fw_boost_g++_-O3_-ffast-math_-march=native_bench-inputs.csv
measurements/data-mhaessig/fw_c-autotuning-tile_clang_-O3_-fno-unroll-loops_-fno-slp-vectorize_bench-inputs.csv
measurements/data-mhaessig/fw_c-autotuning-unroll_clang_-O3_-fno-unroll-loops_-fno-slp-vectorize_bench-inputs.csv
measurements/data-mhaessig/fw_c-autotuning-vector-tile_clang_-O3_-march=native_-ffast-math_bench-inputs.csv
measurements/data-mhaessig/fw_c-autotuning-vector-unroll_clang_-O3_-march=native_-ffast-math_bench-inputs.csv
measurements/data-mhaessig/fw_c-naive_clang_-O3_-fno-tree-vectorize_-fno-slp-vectorize_-fno-unroll-loops_bench-inputs.csv
measurements/data-mhaessig/fw_c-tile_clang_-O3_-fno-tree-vectorize_-fno-slp-vectorize_-fno-unroll-loops_bench-inputs.csv
measurements/data-mhaessig/fw_c-unroll_clang_-O3_-fno-tree-vectorize_-fno-slp-vectorize_-fno-unroll-loops_bench-inputs.csv
measurements/data-mhaessig/fw_c-vector-tile_clang_-O3_-ffast-math_-march=native_bench-inputs.csv
EOM
        ) \
        --plot measurements/plots \
        --title 'Shortest-Path Optimizations' \
        --labels 'impl' \
        --output 'sp-optimizations-large' \
        -vpi=8.0

    python3 measurements/roof-plots.py \
        --data $(
            cat <<EOM
measurements/data-mhaessig/fw_boost_g++_-O3_-ffast-math_-march=native_bench-inputs.csv
measurements/data-mhaessig/fw_c-autotuning-tile_clang_-O3_-fno-unroll-loops_-fno-slp-vectorize_bench-inputs.csv
measurements/data-mhaessig/fw_c-autotuning-unroll_clang_-O3_-fno-unroll-loops_-fno-slp-vectorize_bench-inputs.csv
measurements/data-mhaessig/fw_c-autotuning-vector-tile_clang_-O3_-march=native_-ffast-math_bench-inputs.csv
measurements/data-mhaessig/fw_c-autotuning-vector-unroll_clang_-O3_-march=native_-ffast-math_bench-inputs.csv
measurements/data-mhaessig/fw_c-naive_clang_-O3_-fno-tree-vectorize_-fno-slp-vectorize_-fno-unroll-loops_bench-inputs.csv
measurements/data-mhaessig/fw_c-tile_clang_-O3_-fno-tree-vectorize_-fno-slp-vectorize_-fno-unroll-loops_bench-inputs.csv
measurements/data-mhaessig/fw_c-unroll_clang_-O3_-fno-tree-vectorize_-fno-slp-vectorize_-fno-unroll-loops_bench-inputs.csv
measurements/data-mhaessig/fw_c-vector-tile_clang_-O3_-ffast-math_-march=native_bench-inputs.csv
EOM
        ) \
        --plot measurements/plots \
        --title 'Shortest Path Roofline' \
        --labels impl \
        --output 'sp-optimizations-large' \
        -pi=2.0 \
        -vpi=8.0 \
        -b=7.53

    # mm
    python3 measurements/perf-plots.py \
        --data measurements/data-lasse/*.csv \
        --plot measurements/plots \
        --title 'Min-Max Optimizations' \
        --labels 'impl' \
        --output 'mm-optimizations-large' \
        -vpi=8.0

    python3 measurements/roof-plots.py \
        --data measurements/data-lasse/*.csv \
        --plot measurements/plots \
        --title 'Max-Min Roofline' \
        --labels impl \
        --output 'mm-optimizations-large' \
        -pi=2.0 \
        -b=7.53

    # tc
    python3 measurements/perf-plots.py \
        --data measurements/data-roman/*.csv \
        --plot measurements/plots \
        --title 'Transitive-Closure Optimizations' \
        --labels 'impl' \
        --output 'tc-optimizations-large' \
        --bit-packed \
        -pi=2.0

    python3 measurements/roof-plots.py \
        --data measurements/data-roman/*.csv \
        --plot measurements/plots \
        --title 'Transitive-Closure Roofline' \
        --labels impl \
        --output 'tc-optimizations-large' \
        -pi=2.0 \
        -vpi=64.0 \
        -b=7.03 \
        -bp
}

function clean() {
    make clean
    rm -f $(find "${INPUT_CATEGORY_DIR}" -name "*.out.*")
    rm -f ${MEASUREMENTS_DIR}/*.csv # to prevent recursing into more directories
    rm -f $(find "${PLOTS_DIR}" -name "*test-inputs*")
    rm -f $(find "${GENERIC_DIR}/c/impl" -name "*.c")
}

COMMAND=${1:-}
if [[ -z "$COMMAND" ]]; then
    printUsage "$0"
fi

case "$COMMAND" in
build)
    ALGORITHM_LIST="${2:-}"
    IMPLEMENTATION_LIST="${3:-}"
    COMPILER_LIST="${4:-}"
    OPTIMIZATIONS_LIST="${5:-}"
    if [[ -z "$ALGORITHM_LIST" || -z "$IMPLEMENTATION_LIST" || -z "$COMPILER_LIST" || -z "$OPTIMIZATIONS_LIST" ]]; then
        printUsage "$0"
    fi

    IFS=','
    read -ra ALGORITHMS <<<"$ALGORITHM_LIST"
    read -ra IMPLEMENTATIONS <<<"$IMPLEMENTATION_LIST"
    read -ra COMPILERS <<<"$COMPILER_LIST"
    read -ra OPTS <<<"$OPTIMIZATIONS_LIST"

    for ALGORITHM in "${ALGORITHMS[@]}"; do
        for IMPLEMENTATION in "${IMPLEMENTATIONS[@]}"; do
            for COMPILER in "${COMPILERS[@]}"; do
                for OPTIMIZATIONS in "${OPTS[@]}"; do
                    echo "Building $ALGORITHM with '$COMPILER' and '$OPTIMIZATIONS'"
                    build "$ALGORITHM" "$IMPLEMENTATION" "$COMPILER" "$OPTIMIZATIONS"
                    echo
                done
            done
        done
    done
    ;;
generate-test-in)
    MIN_N="${2:-}"
    MAX_N="${3:-}"
    STEP_N="${4:-}"
    if [[ -z "$MIN_N" || -z "$MAX_N" || -z "$STEP_N" ]]; then
        printUsage "$0"
    fi
    echo "Generating graphs from $MIN_N to $MAX_N with step $STEP_N"
    generate-test-in "$MIN_N" "$MAX_N" "$STEP_N"
    echo
    ;;
generate-test-out)
    ALGORITHM="${2:-}"
    REF_IMPL="${3:-}"
    INPUT_CATEGORY="${4:-}"
    if [[ -z "$ALGORITHM" || -z "$REF_IMPL" || -z "$INPUT_CATEGORY" ]]; then
        printUsage "$0"
    fi
    echo "Generating expected ouput of input category $INPUT_CATEGORY with $ALGORITHM/$REF_IMPL"
    generate-test-out "$ALGORITHM" "$REF_IMPL" "$INPUT_CATEGORY"
    echo
    ;;
validate)
    ALGORITHM_LIST="${2:-}"
    IMPLEMENTATION_LIST="${3:-}"
    COMPILER_LIST="${4:-}"
    OPTIMIZATIONS_LIST="${5:-}"
    TESTCASE_LIST="${6:-/}"
    if [[ -z "$ALGORITHM_LIST" || -z "$IMPLEMENTATION_LIST" || -z "$COMPILER_LIST" || -z "$OPTIMIZATIONS_LIST" ]]; then
        printUsage "$0"
    fi

    IFS=','
    read -ra ALGORITHMS <<<"$ALGORITHM_LIST"
    read -ra IMPLEMENTATIONS <<<"$IMPLEMENTATION_LIST"
    read -ra COMPILERS <<<"$COMPILER_LIST"
    read -ra OPTS <<<"$OPTIMIZATIONS_LIST"
    read -ra TESTCASES <<<"$TESTCASE_LIST"

    for ALGORITHM in "${ALGORITHMS[@]}"; do
        for IMPLEMENTATION in "${IMPLEMENTATIONS[@]}"; do
            for COMPILER in "${COMPILERS[@]}"; do
                for OPTIMIZATIONS in "${OPTS[@]}"; do
                    for TESTCASE in "${TESTCASES[@]}"; do
                        echo "Validating $ALGORITHM/$IMPLEMENTATION"
                        validate $ALGORITHM "$IMPLEMENTATION" "$COMPILER" "$OPTIMIZATIONS" "$TESTCASE_DIR/$TESTCASE"
                        echo
                    done
                done
            done
        done
    done
    ;;
measure)
    ALGORITHM_LIST="${2:-}"
    IMPLEMENTATION_LIST="${3:-}"
    COMPILER_LIST="${4:-}"
    OPTIMIZATIONS_LIST="${5:-}"
    INPUT_CATEGORY="${6:-bench-inputs}"
    TESTCASES="${7:-}"
    if [[ -z "$ALGORITHM_LIST" || -z "$IMPLEMENTATION_LIST" || -z "$COMPILER_LIST" || -z "$OPTIMIZATIONS_LIST" || -z "$INPUT_CATEGORY" ]]; then
        printUsage "$0"
    fi

    IFS=','
    read -ra ALGORITHMS <<<"$ALGORITHM_LIST"
    read -ra IMPLEMENTATIONS <<<"$IMPLEMENTATION_LIST"
    read -ra COMPILERS <<<"$COMPILER_LIST"
    read -ra OPTS <<<"$OPTIMIZATIONS_LIST"

    for ALGORITHM in "${ALGORITHMS[@]}"; do
        for IMPLEMENTATION in "${IMPLEMENTATIONS[@]}"; do
            for COMPILER in "${COMPILERS[@]}"; do
                for OPTIMIZATIONS in "${OPTS[@]}"; do
                    echo "Measuring $ALGORITHM/$IMPLEMENTATION compiled with '$COMPILER' and '$OPTIMIZATIONS' on '$INPUT_CATEGORY'"
                    measure "$ALGORITHM" "$IMPLEMENTATION" "$COMPILER" "$OPTIMIZATIONS" "$INPUT_CATEGORY" "$TESTCASES"
                    echo
                done
            done
        done
    done
    ;;
plot)
    ALGORITHM_LIST="${2:-}"
    IMPLEMENTATION_LIST="${3:-}"
    COMPILER_LIST="${4:-}"
    OPTIMIZATIONS_LIST="${5:-}"
    INPUT_CATEGORY="${6:-bench-inputs}"
    PLOT_TITLE="${7:-}"
    PLOT_LABELS_LIST="${8:-algo,impl,comp,opts,testsuite}"
    if [[ -z "$ALGORITHM_LIST" || -z "$IMPLEMENTATION_LIST" || -z "$COMPILER_LIST" || -z "$OPTIMIZATIONS_LIST" || -z "$PLOT_TITLE" ]]; then
        printUsage "$0"
    fi

    IFS=','
    read -ra ALGORITHMS <<<"$ALGORITHM_LIST"
    read -ra IMPLEMENTATIONS <<<"$IMPLEMENTATION_LIST"
    read -ra COMPILERS <<<"$COMPILER_LIST"
    read -ra OPTS <<<"$OPTIMIZATIONS_LIST"
    read -ra PLOT_LABELS <<<"$PLOT_LABELS_LIST"

    for ALGORITHM in "${ALGORITHMS[@]}"; do
        for IMPLEMENTATION in "${IMPLEMENTATIONS[@]}"; do
            for COMPILER in "${COMPILERS[@]}"; do
                for OPTIMIZATIONS in "${OPTS[@]}"; do
                    echo "Plotting $ALGORITHM/$IMPLEMENTATION compiled with $COMPILER and $OPTIMIZATIONS as $PLOT_TITLE"
                    plot "$ALGORITHM" "$IMPLEMENTATION" "$COMPILER" "$OPTIMIZATIONS" "$INPUT_CATEGORY" "$PLOT_TITLE" "$PLOT_LABELS"
                    echo
                done
            done
        done
    done
    ;;
plot-final)
    echo "Plotting"
    plot-final
    echo
    ;;
clean)
    echo "Cleaning"
    clean
    echo
    ;;
esac

exit
