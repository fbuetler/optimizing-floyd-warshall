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

function printUsage() {
    echo "Usage: $0"
    echo "Commands:"
    echo "  build <ALGORITHM|REF_IMPL>"
    echo "  generate-test-in <MIN_N> <MAX_N> <STEP_N>"
    echo "  generate-test-out <REF_IMPL> <INPUT_CATEGORY>"
    echo "  validate <ALGORITHM> <COMPILER>"
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
    echo "Input categories:"
    echo "  $(ls -m $INPUT_CATEGORY_DIR | sed 's/,/\n /g')"
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
    INPUT_CATEGORY="$2"
    TESTCASES=$(find ${INPUT_CATEGORY_DIR}/${INPUT_CATEGORY} -name "*.in.txt")
    # TODO generate mm.ref and tc.ref too
    for TESTCASE_IN in $TESTCASES; do
        "${BUILD_DIR}/$REF_IMPL" \
            -input-filename "${TESTCASE_IN}" \
            -output-filename "${TESTCASE_IN%%.in.txt}.fw.ref.txt"
    done
}

function validate() {
    ALGORITHM="$1"
    COMPILER="$2"
    # TODO generate mm.out and tc.out too
    python3 "${ROOT_DIR}/comparator/runner.py" \
        -b "${BUILD_DIR}/${ALGORITHM}_${COMPILER}" \
        -d "${TESTCASE_DIR}" \
        -a "fw" \
        -o "out"

    python3 "${ROOT_DIR}/comparator/compare.py" \
        --recursive \
        --silent \
        "${TESTCASE_DIR}"
}

function measure() {
    ALGORITHM="$1"
    COMPILER="$2"
    python3 "${ROOT_DIR}/measurements/measure.py" \
        --binary "${BUILD_DIR}/${ALGORITHM}_${COMPILER}" \
        --testsuite "${BENCHMARK_DIR}" \
        --output "${MEASUREMENTS_DIR}/${ALGORITHM}_${COMPILER}"
}

function plot() {
    ALGORITHM="$1"
    COMPILER="$2"
    PLOT_TITLE="$3"
    python3 "${ROOT_DIR}/measurements/perf-plots.py" \
        --data "${MEASUREMENTS_DIR}/${ALGORITHM}_${COMPILER}.csv" \
        --plot "${PLOTS_DIR}" \
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
    INPUT_CATEGORY="${3:-}"
    if [[ -z "$REF_IMPL" || -z "$INPUT_CATEGORY" ]]; then
        printUsage "$0"
    fi
    echo "Generating expected ouput of input category $INPUT_CATEGORY with $REF_IMPL"
    generate-test-out "$REF_IMPL" "$INPUT_CATEGORY"
    echo
    ;;
validate)
    ALGORITHM="${2:-}"
    COMPILER="${3:-}"
    if [[ -z "$ALGORITHM" || -z "$COMPILER" ]]; then
        printUsage "$0"
    fi
    echo "Validating $ALGORITHM"
    validate $ALGORITHM $COMPILER
    echo
    ;;
measure)
    ALGORITHM="${2:-}"
    COMPILER="${3:-}"
    if [[ -z "$ALGORITHM" || -z "$COMPILER" ]]; then
        printUsage "$0"
    fi
    echo "Measuring $ALGORITHM compiled with $COMPILER"
    measure "$ALGORITHM" "$COMPILER"
    echo
    ;;
plot)
    ALGORITHM="${2:-}"
    COMPILER="${3:-}"
    PLOT_TITLE="${4:-}"
    if [[ -z "$ALGORITHM" || -z "$COMPILER" || -z "$PLOT_TITLE" ]]; then
        printUsage "$0"
    fi
    echo "Plotting $ALGORITHM compiled with $COMPILER as $PLOT_TITLE"
    plot "$ALGORITHM" "$COMPILER" "$PLOT_TITLE"
    echo
    ;;
clean)
    echo "Cleaning"
    clean
    echo
    ;;
esac

exit
