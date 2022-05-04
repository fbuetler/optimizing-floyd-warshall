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
    echo "  build <ALGORITHM> <IMPLEMENTATION> <OPTIMIZATIONS>"
    echo "  generate-test-in <MIN_N> <MAX_N> <STEP_N>"
    echo "  generate-test-out <ALGORITHM> <REF_IMPL> <INPUT_CATEGORY>"
    echo "  validate <ALGORITHM> <IMPLEMENTATION> <COMPILER> <OPTIMIZATIONS>"
    echo "  measure <ALGORITHM> <IMPLEMENTATION> <COMPILER> <OPTIMIZATIONS> (<INPUT_CATEGORY>)"
    echo "  plot <ALGORITHM> <IMPLEMENTATION> <COMPILER> <OPTIMIZATIONS> <PLOT_TITLE>"
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
    echo "Optimization examples:"
    echo "  -O0"
    echo "  -O3 -fno-tree-vectorize"
    echo "  -O3 -ffast-math -march=native -mfma"
    echo "Input categories:"
    echo "  $(ls -m $INPUT_CATEGORY_DIR | sed 's/,/\n /g')"
    exit 1
}

function optimizations_format() {
    OPTIMIZATIONS_RAW="$1"
    echo $OPTIMIZATIONS_RAW | tr " " "_"
}

function build() {
    ALGORITHM="$1"
    IMPLEMENTATION="$2"
    OPTIMIZATIONS_RAW="$3"

    make "build-${ALGORITHM}-${IMPLEMENTATION}" -e CFLAGS_DOCKER="$OPTIMIZATIONS_RAW"

    # HACK: filenames are at the moment hardcoded in the docker make file
    OPTIMIZATIONS=$(optimizations_format "$OPTIMIZATIONS_RAW")
    echo $OPTIMIZATIONS
    find ./build \
        -type f \
        -regex ".*/${ALGORITHM}_${IMPLEMENTATION}_\(gcc\|clang\)$" \
        -exec mv --force {} {}_$OPTIMIZATIONS \;
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
    # TODO generate mm.out and tc.out too
    python3 "${ROOT_DIR}/comparator/runner.py" \
        -b "${BUILD_DIR}/${ALGORITHM}_${IMPLEMENTATION}_${COMPILER}_$OPTIMIZATIONS" \
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
    OPTIMIZATIONS_RAW="$4"
    OPTIMIZATIONS=$(optimizations_format "$OPTIMIZATIONS_RAW")
    INPUT_CATEGORY="$5"
    python3 "${ROOT_DIR}/measurements/measure.py" -tb \
        --binary "${BUILD_DIR}/${ALGORITHM}_${IMPLEMENTATION}_${COMPILER}_$OPTIMIZATIONS" \
        --testsuite "${INPUT_CATEGORY_DIR}/${INPUT_CATEGORY}" \
        --output "${MEASUREMENTS_DIR}/${ALGORITHM}_${IMPLEMENTATION}_${COMPILER}_$OPTIMIZATIONS"
}

function plot() {
    ALGORITHM="$1"
    IMPLEMENTATION="$2"
    COMPILER="$3"
    OPTIMIZATIONS_RAW="$4"
    OPTIMIZATIONS=$(optimizations_format "$OPTIMIZATIONS_RAW")
    PLOT_TITLE="$5"
    python3 "${ROOT_DIR}/measurements/perf-plots.py" \
        --data "${MEASUREMENTS_DIR}/${ALGORITHM}_${IMPLEMENTATION}_${COMPILER}_$OPTIMIZATIONS.csv" \
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
    OPTIMIZATIONS="${4:-}"
    if [[ -z "$ALGORITHM" || -z "$IMPLEMENTATION" || -z "$OPTIMIZATIONS" ]]; then
        printUsage "$0"
    fi
    echo "Building $ALGORITHM"
    build "$ALGORITHM" "$IMPLEMENTATION" "$OPTIMIZATIONS"
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
    OPTIMIZATIONS="${5:-}"
    if [[ -z "$ALGORITHM" || -z "$IMPLEMENTATION" || -z "$COMPILER" || -z "$OPTIMIZATIONS" ]]; then
        printUsage "$0"
    fi
    echo "Validating $ALGORITHM/$IMPLEMENTATION"
    validate $ALGORITHM "$IMPLEMENTATION" "$COMPILER" "$OPTIMIZATIONS"
    echo
    ;;
measure)
    ALGORITHM="${2:-}"
    IMPLEMENTATION="${3:-}"
    COMPILER="${4:-}"
    OPTIMIZATIONS="${5:-}"
    INPUT_CATEGORY="${6:-bench-inputs}"
    if [[ -z "$ALGORITHM" || -z "$IMPLEMENTATION" || -z "$COMPILER" || -z "$OPTIMIZATIONS" || -z "$INPUT_CATEGORY" ]]; then
        printUsage "$0"
    fi
    echo "Measuring $ALGORITHM/$IMPLEMENTATION compiled with $COMPILER and $OPTIMIZATIONS on $INPUT_CATEGORY"
    measure "$ALGORITHM" "$IMPLEMENTATION" "$COMPILER" "$OPTIMIZATIONS" "$INPUT_CATEGORY"
    echo
    ;;
plot)
    ALGORITHM="${2:-}"
    IMPLEMENTATION="${3:-}"
    COMPILER="${4:-}"
    OPTIMIZATIONS="${5:-}"
    PLOT_TITLE="${6:-}"
    if [[ -z "$ALGORITHM" || -z "$IMPLEMENTATION" || -z "$COMPILER" || -z "$OPTIMIZATIONS" || -z "$PLOT_TITLE" ]]; then
        printUsage "$0"
    fi
    echo "Plotting $ALGORITHM/$IMPLEMENTATION compiled with $COMPILER and $OPTIMIZATIONS as $PLOT_TITLE"
    plot "$ALGORITHM" "$IMPLEMENTATION" "$COMPILER" "$OPTIMIZATIONS" "$PLOT_TITLE"
    echo
    ;;
clean)
    echo "Cleaning"
    clean
    echo
    ;;
esac

exit
