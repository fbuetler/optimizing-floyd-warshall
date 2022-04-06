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
    echo "  build <ALGORITHM> <IMPLEMENTATION>"
    echo "  generate-test-in <MIN_N> <MAX_N> <STEP_N>"
    echo "  generate-test-out <ALGORITHM> <REF_IMPL> <INPUT_CATEGORY>"
    echo "  validate <ALGORITHM> <IMPLEMENTATION> <COMPILER>"
    echo "  measure <ALGORITHM> <IMPLEMENTATION> <COMPILER> (<INPUT_CATEGORY>)"
    echo "  plot <ALGORITHM> <IMPLEMENTATION> <COMPILER> <TESTCASE> <PLOT_TITLE>"
    echo "  clean"
    echo "Algorithms:"
    echo "  fw (floyd-wahrshal)"
    echo "  tc (transitive closure)"
    echo "  mm (max-min)"
    echo "Implementations:"
    echo "  c-naive"
    echo "Reference implementations:"
    echo "  go-ref"
    echo "Compilers:"
    echo "  gcc"
    echo "  clang"
    echo "Input categories:"
    echo "  $(ls -m $INPUT_CATEGORY_DIR | sed 's/,/\n /g')"
    exit 1
}

function build() {
    ALGORITHM="$1"
    IMPLEMENTATION="$2"
    make "build-${ALGORITHM}-${IMPLEMENTATION}"
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
    # TODO generate mm.out and tc.out too
    python3 "${ROOT_DIR}/comparator/runner.py" \
        -b "${BUILD_DIR}/${ALGORITHM}_${IMPLEMENTATION}_${COMPILER}" \
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
    IMPLEMENTATION="$2"
    COMPILER="$3"
    INPUT_CATEGORY="$4"
    python3 "${ROOT_DIR}/measurements/measure.py" \
        --binary "${BUILD_DIR}/${ALGORITHM}_${IMPLEMENTATION}_${COMPILER}" \
        --testsuite "${INPUT_CATEGORY_DIR}/${INPUT_CATEGORY}" \
        --output "${MEASUREMENTS_DIR}/${ALGORITHM}_${IMPLEMENTATION}_${COMPILER}"
}

function plot() {
    ALGORITHM="$1"
    IMPLEMENTATION="$2"
    COMPILER="$3"
    PLOT_TITLE="$4"
    python3 "${ROOT_DIR}/measurements/perf-plots.py" \
        --data "${MEASUREMENTS_DIR}/${ALGORITHM}_${IMPLEMENTATION}_${COMPILER}.csv" \
        --plot "${PLOTS_DIR}" \
        --title "$PLOT_TITLE"
}

function clean() {
    make clean
    rm $(find "${INPUT_CATEGORY_DIR}" -name "*.out.*")
}

COMMAND=${1:-}
if [[ -z "$COMMAND" ]]; then
    printUsage "$0"
fi

case "$COMMAND" in
build)
    ALGORITHM="${2:-}"
    IMPLEMENTATION="${3:-}"
    if [[ -z "$ALGORITHM" || -z "$IMPLEMENTATION" ]]; then
        printUsage "$0"
    fi
    echo "Building $ALGORITHM"
    build "$ALGORITHM" "$IMPLEMENTATION"
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
    ALGORITHM="${2:-}"
    IMPLEMENTATION="${3:-}"
    COMPILER="${4:-}"
    if [[ -z "$ALGORITHM" || -z "$IMPLEMENTATION" || -z "$COMPILER" ]]; then
        printUsage "$0"
    fi
    echo "Validating $ALGORITHM/$IMPLEMENTATION"
    validate $ALGORITHM "$IMPLEMENTATION" $COMPILER
    echo
    ;;
measure)
    ALGORITHM="${2:-}"
    IMPLEMENTATION="${3:-}"
    COMPILER="${4:-}"
    INPUT_CATEGORY="${5:-bench-inputs}"
    if [[ -z "$ALGORITHM" || -z "$IMPLEMENTATION" || -z "$COMPILER" || -z "$INPUT_CATEGORY" ]]; then
        printUsage "$0"
    fi
    echo "Measuring $ALGORITHM/$IMPLEMENTATION compiled with $COMPILER on $INPUT_CATEGORY"
    measure "$ALGORITHM" "$IMPLEMENTATION" "$COMPILER" "$INPUT_CATEGORY"
    echo
    ;;
plot)
    ALGORITHM="${2:-}"
    IMPLEMENTATION="${3:-}"
    COMPILER="${4:-}"
    PLOT_TITLE="${5:-}"
    if [[ -z "$ALGORITHM" || -z "$IMPLEMENTATION" || -z "$COMPILER" || -z "$PLOT_TITLE" ]]; then
        printUsage "$0"
    fi
    echo "Plotting $ALGORITHM/$IMPLEMENTATION compiled with $COMPILER as $PLOT_TITLE"
    plot "$ALGORITHM" "$IMPLEMENTATION" "$COMPILER" "$PLOT_TITLE"
    echo
    ;;
clean)
    echo "Cleaning"
    clean
    echo
    ;;
esac

exit
