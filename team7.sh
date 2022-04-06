#!/bin/bash

# Safer bash scripts
# @see https://explainshell.com/explain?cmd=set+-euCo+pipefail
# @see https://vaneyckt.io/posts/safer_bash_scripts_with_set_euxo_pipefail/
set -euCo pipefail

# Configuration variables
ROOT_DIR=$(dirname "$0")
BUILD_DIR="${ROOT_DIR}/build"
TESTCASE_DIR="${ROOT_DIR}/testcases"
MEASUREMENT_DATA_DIR="${ROOT_DIR}/measurements/data"
MEASUREMENT_PLOT_DIR="${ROOT_DIR}/measurements/plots"

function printUsage() {
    echo "Usage: $0"
    echo "Commands:"
    echo "  build <ALGORITHM|REF_IMPL>"
    echo "  generate-test-in <MIN_N> <MAX_N> <STEP_N>"
    echo "  generate-test-out <REF_IMPL> <TESTCASE>"
    echo "  run <ALGORITHM> <COMPILER>"
    echo "  compare"
    echo "  measure <ALGORITHM> <COMPILER> <TESTCASE>"
    echo "  plot <ALGORITHM> <COMPILER> <TESTCASE> <PLOT_TITLE>"
    echo "  clean"
    echo "Algorithms:"
    echo "  c-naive-shortest-path"
    echo "  c-naive-transitive-closure"
    echo "  c-naive-max-min"
    echo "Compilers:"
    echo "  gcc"
    echo "  clang"
    echo "Testcases:"
    echo "  see $TESTCASE_DIR"
    echo "Reference implementations:"
    echo "  go-ref-shortest-path"
    exit 1
}

function build() {
    ALGORITHM="$1"
    make "build-$ALGORITHM"
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
    REF_IMPL="$1"
    TESTCASE="$2"
    TESTCASES=$(find ${TESTCASE_DIR}/${TESTCASE} -name "*.in.txt")
    for TESTCASE_IN in $TESTCASES; do
        "${BUILD_DIR}/$REF_IMPL" \
            -input-filename "${TESTCASE_IN}" \
            -output-filename "${TESTCASE_IN%%.in.txt}.fw.ref.txt"
    done
}

function run() {
    ALGORITHM="$1"
    COMPILER="$2"
    python3 "${ROOT_DIR}/comparator/runner.py" \
        -b "${BUILD_DIR}/${ALGORITHM}_${COMPILER}" \
        -d "${TESTCASE_DIR}"
}

function compare() {
    python3 "${ROOT_DIR}/comparator/compare.py" -r "${TESTCASE_DIR}"
}

function measure() {
    ALGORITHM="$1"
    COMPILER="$2"
    TESTCASE="$3"
    python3 "${ROOT_DIR}/measurements/measure.py" \
        --binary "${BUILD_DIR}/${ALGORITHM}_${COMPILER}" \
        --testsuite "${TESTCASE_DIR}/$TESTCASE" \
        --measurements "${MEASUREMENT_DATA_DIR}"
}

function plot() {
    ALGORITHM="$1"
    COMPILER="$2"
    TESTCASE="$3"
    PLOT_TITLE="$4"
    python3 "${ROOT_DIR}/measurements/plot.py" \
        --data "${MEASUREMENT_DATA_DIR}/${ALGORITHM}_${COMPILER}_${TESTCASE}.csv" \
        --plot "${MEASUREMENT_PLOT_DIR}" \
        --title "$PLOT_TITLE"
}

function clean() {
    make clean
}

COMMAND=${1:-}
if [[ -z "$COMMAND" ]]; then
    printUsage "$0"
fi

case "$COMMAND" in
build)
    ALGORITHM="${2:-}"
    if [[ -z "$ALGORITHM" ]]; then
        printUsage "$0"
    fi
    echo "Building $ALGORITHM"
    build "$ALGORITHM"
    echo
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
    REF_IMPL="${2:-}"
    TESTCASE="${3:-}"
    if [[ -z "$REF_IMPL" || -z "$TESTCASE" ]]; then
        printUsage "$0"
    fi
    echo "Generating expected ouput of testcase category $TESTCASE with $REF_IMPL"
    generate-test-out "$REF_IMPL" "$TESTCASE"
    echo
    ;;
run)
    ALGORITHM="${2:-}"
    COMPILER="${3:-}"
    if [[ -z "$ALGORITHM" || -z "$COMPILER" ]]; then
        printUsage "$0"
    fi
    echo "Run $ALGORITHM compiled with $COMPILER against testcases"
    run "$ALGORITHM" "$COMPILER"
    echo
    ;;
compare)
    echo "Comparing actual output against expected output"
    compare
    echo
    ;;
measure)
    ALGORITHM="${2:-}"
    COMPILER="${3:-}"
    TESTCASE="${4:-}"
    if [[ -z "$ALGORITHM" || -z "$COMPILER" || -z "$TESTCASE" ]]; then
        printUsage "$0"
    fi
    echo "Measuring $ALGORITHM compiled with $COMPILER on $TESTCASE"
    measure "$ALGORITHM" "$COMPILER" "$TESTCASE"
    echo
    ;;
plot)
    ALGORITHM="${2:-}"
    COMPILER="${3:-}"
    TESTCASE="${4:-}"
    PLOT_TITLE="${5:-}"
    if [[ -z "$ALGORITHM" || -z "$COMPILER" || -z "$TESTCASE" || -z "$PLOT_TITLE" ]]; then
        printUsage "$0"
    fi
    echo "Plotting $ALGORITHM compiled with $COMPILER on $TESTCASE as $PLOT_TITLE"
    plot "$ALGORITHM" "$COMPILER" "$TESTCASE" "$PLOT_TITLE"
    echo
    ;;
clean)
    echo "Cleaning"
    clean
    echo
    ;;
esac

exit
