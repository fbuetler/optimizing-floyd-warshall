import argparse
import math
from os import path
from typing import List, Tuple, Callable
import jinja2
import subprocess
import logging
import csv

DEBUG = False
VALIDATING = True

logging.basicConfig(level=logging.DEBUG) if DEBUG else logging.basicConfig(level=logging.INFO)

ALGORITHM_FW = "fw"
ALGORITHM_MM = "mm"
ALGORITHM_TC = "tc"

ALGORITHM_CHOICES = {
    ALGORITHM_FW,
    ALGORITHM_MM,
    ALGORITHM_TC,
}

parser = argparse.ArgumentParser()
parser.add_argument(
    "-p", "--project-root", help="path of the project root", type=str, required=True
)
# parser.add_argument(
#     "-l1", "--l1-cache", help="size of the L1 cache in bytes", type=int, required=True
# )
parser.add_argument(
    "-l2", "--l2-cache", help="size of the L2 cache in bytes", type=int, required=True
)
parser.add_argument(
    "-algo",
    "--algorithm",
    help="algorithm to autotune",
    choices=ALGORITHM_CHOICES,
    required=True,
)
parser.add_argument("-n", "--input-size", help="input size", type=int, required=True)
# parser.add_argument("-n", "--min-n", help="minimum input size", type=int, required=True)
# parser.add_argument("-N", "--max-n", help="maximum input size", type=int, required=True)
# parser.add_argument(
#     "-vec",
#     "--vectorize",
#     help="generated code should use vector instructions",
#     action="store_true",
# )

IMPLEMENTATION = "c-autotune"
COMPILER = "clang"
OPT_FLAGS = "-O3 -fno-tree-vectorize"
C_FLAGS_SCALAR = "-O3 -fno-slp-vectorize"
C_FLAGS_VECTOR = "-O3 -march=native -ffast-math"
TEST_INPUT = "test-inputs"
BENCH_INPUT = "bench-inputs"
BENCH_INPUT_DIR = "bench-inputs"
TEMPLATE_DIR = 'autotuning/templates'
SOURCE_DIR = 'generic/c/impl'
DATA_DIR = 'measurements/data'


def clean_files(project_root):
    clean_cmd = [
        "bash",
        f"{project_root}/team7.sh",
        "clean",
    ]

    logging.debug(" ".join(clean_cmd))
    result = subprocess.run(clean_cmd)
    return result.returncode


def render_jinja_template(template_loc: str, file_name: str, **context) -> str:
    """ generates text from a jinja template using the given context and returns the generated text as a string """
    return (
        jinja2.Environment(loader=jinja2.FileSystemLoader(template_loc + "/"))
        .get_template(file_name)
        .render(context)
    )

def generate_fwi(template_dir: str, output_dir: str, algorithm: str, form: str, vectorized: bool, compiler: str, c_flags: str, parameters: Tuple[int, int]) -> str:
    """ Generates a FW source file and returns the implementation and more specific (parametrized) name.

        Form needs to be one of 'FWI', 'FWIabc' or 'FWT', requiring parameters
        (Ui,Uj), (Ui',Uj',Uk') or (L1,Ui,Uj,Ui',Uj',Uk'), respectively.
    """

    logging.info(f"generating source: {form} with {parameters}")

    context = dict()
    if form == 'FWI':
        template_file = 'fw-unroll.py.j2'
        (ui, uj) = parameters
        implementation = 'vector-unroll' if vectorized else 'unroll'
        param_implementation = f'{implementation}-{ui}-{uj}'
        output_fname = f"{output_dir}/{algorithm}_{param_implementation}_{compiler}_{c_flags.replace(' ', '_')}.c"
        context['unroll_i'] = ui
        context['unroll_j'] = uj
    elif form == 'FWIabc':
        template_file = 'fw-unroll-abc.py.j2'
        (uii, ujj, ukk) = parameters
        # HACK: Same filename as FWI file. Shouldn't matter because we only use this to find an initial value
        implementation = 'vector-unroll' if vectorized else 'unroll'
        param_implementation = f'{implementation}-{ui}-{uj}'
        output_fname = f"{output_dir}/{algorithm}_{param_implementation}_{compiler}_{c_flags.replace(' ', '_')}.c"
        context['unroll_ii'] = uii
        context['unroll_jj'] = ujj
        context['unroll_kk'] = ukk
    elif form == 'FWT':
        template_file = 'fw-tile.py.j2'
        (l1, ui, uj, uii, ujj, ukk) = parameters
        implementation = 'vector-tiles' if vectorized else 'tile'
        param_implementation = f'{implementation}-{l1}-{ui}-{uj}-{uii}-{ujj}-{ukk}'
        output_fname = f"{output_dir}/{algorithm}_{param_implementation}_{compiler}_{c_flags.replace(' ', '_')}.c"
        context['L1'] = l1
        context['unroll_i'] = ui
        context['unroll_j'] = uj
        context['unroll_ii'] = uii
        context['unroll_jj'] = ujj
        context['unroll_kk'] = ukk
    else:
        raise Exception(f'Unrecognized form {form} - Needs to be one of FWI, FWIabc, or FWT')

    if algorithm == ALGORITHM_FW:
        context["algorithm"] = "sp"
        context["datatype"] = "double"
        context["outer_op"] = "MIN"
        context["inner_op"] = "ADD"
    elif algorithm == ALGORITHM_MM:
        context["algorithm"] = "mm"
        context["datatype"] = "double"
        context["datatype"] = "double"
        context["outer_op"] = "MAX"
        context["inner_op"] = "MIN"
    elif algorithm == ALGORITHM_TC:
        context["algorithm"] = "tc"
        context["datatype"] = "char"
        context["outer_op"] = "OR"
        context["inner_op"] = "AND"

    with open(output_fname, mode="w", encoding="utf-8") as f:
            f.write(
                render_jinja_template(
                    f"{template_dir}",
                    template_file,
                    **context,
                )
            )

    return implementation, param_implementation

def build_files(project_root: str, algorithm: str, implementation: str, compiler: str, c_flags: str) -> int:
    """ builds source file(s) as specified by the given parameters """

    build_cmd = [
        "make",
        "-C",
        f"{project_root}",
        f"build-{algorithm}-{implementation}-{compiler}",
        "-e",
        f"CFLAGS_DOCKER={c_flags}",
    ]

    logging.debug(" ".join(build_cmd))
    result = subprocess.run(build_cmd)
    return result.returncode

def validate_fwi(
    project_root: str, algorithm: str, p_impl: str, n_factor: int, compiler: str, c_flags: str
) -> int:
    logging.info(f"validating code for: {p_impl}")

    testcases = list()
    for n in [4, 8, 16, 30, 32, 512, 1152]:
        if n % n_factor == 0:
            testcases.append(f"n{n}")

    if len(testcases) == 0:
        logging.warning("Skipping validation as there are no fiting testcases")
        return 0

    validate_cmd = [
        "bash",
        f"{project_root}/team7.sh",
        "validate",
        f"{algorithm}",
        f"{p_impl}",
        f"{compiler}",
        f"{c_flags}",
        f"{','.join(testcases)}",
    ]

    logging.debug(" ".join(validate_cmd))
    result = subprocess.run(
        validate_cmd,
        text=True,
        capture_output=True,
    )

    logging.debug(result.stdout)
    if result.returncode != 0:
        logging.error(result.stderr)
        return result.returncode

    return 0

def measure_fwi(
    project_root: str,
    algorithm: str,
    implementation: str,
    compiler: str,
    c_flags: str,
    test_input_dir: str,
    input_size: int,
) -> int:
    """ run a generated and built implementation against a testcase of the given size """
    logging.info(f"measuring code for: {implementation}")

    measure_cmd = [
        "bash",
        f"{project_root}/team7.sh",
        "measure",
        f"{algorithm}",
        f"{implementation}",
        f"{compiler}",
        f"{c_flags}",
        f"{test_input_dir}",
        f"n{input_size}",
    ]

    logging.debug(" ".join(measure_cmd))
    result = subprocess.run(measure_cmd, capture_output=False, text=True)

    logging.debug(result.stdout)
    if result.returncode != 0:
        logging.error(result.stderr)
        return result.returncode

    return 0

def get_perf(
    data_dir: str,
    algorithm: str,
    p_impl: str,
    compiler: str,
    c_flags: str,
    test_input_dir: str,
    input_size: str,
) -> float:
    """ finds the performance for a specific implementation incl. parameters """

    logging.info(f"processing data: {p_impl}")
    data_fname = f"{data_dir}/{algorithm}_{p_impl}_{compiler}_{c_flags.replace(' ', '_')}_{test_input_dir}.csv"
    with open(data_fname) as f:
        reader = csv.reader(f, delimiter=",", quoting=csv.QUOTE_NONNUMERIC)
        n_list = reader.__next__()
        _ = reader.__next__() # number of runs
        cycles_list = reader.__next__()

        n_index = -1
        for i, n in enumerate(n_list):
            if n == input_size:
                n_index = i
                break

        if n_index == -1:
            logging.error(f"data file didn't contain measurements for the desired input n{input_size}")
            raise Exception("failed to get optimal perf")

        cycles = cycles_list[n_index]
        p = round((2 * n * n * n) / cycles, 2)

        logging.debug(
            f"performance for {p_impl}: {p} flops/cycle"
        )

    return p

def find_best_neighbour(project_root: str, algorithm: str, form: str, vectorized: bool, input_size: int, test_input_dir: str, parameter_list: List[Tuple]) -> Tuple[Tuple, float]:
    """ Takes a list of parameters (neighbours), and generates, builds and measures the performance of source files for each.
        The function returns the optimal parameters and the corresponding performance value.
    """
    c_flags = C_FLAGS_VECTOR if vectorized else C_FLAGS_SCALAR

    # generate source and store in generic/c/impl
    param_impl_list = list()
    for parameters in parameter_list:
        # impl should be the same for each generated source file
        impl, param_impl = generate_fwi(path.join(project_root, TEMPLATE_DIR), path.join(project_root, SOURCE_DIR), algorithm, form, vectorized, COMPILER, c_flags, parameters)
        param_impl_list.append((parameters, param_impl))

    # build source
    retcode = build_files(project_root, algorithm, impl, COMPILER, c_flags)
    if retcode != 0:
        raise Exception(f"Building {impl} failed")

    max_perf = -1

    for params, p_impl in param_impl_list:
        if VALIDATING:
            # validate correctness of neighbour
            retcode = validate_fwi(project_root, algorithm, p_impl, 0, COMPILER, c_flags)
            if retcode != 0:
                raise Exception(f"Validating {p_impl} failed")

        # measure performance
        retcode = measure_fwi(project_root, algorithm, p_impl, COMPILER, c_flags, test_input_dir, input_size)
        if retcode != 0:
            raise Exception(f"Running {p_impl} failed")

        # find highest performance
        perf = get_perf(path.join(project_root, DATA_DIR), algorithm, p_impl, COMPILER, c_flags, test_input_dir, input_size)
        if perf > max_perf:
            max_perf = perf
            best_neighbour = params

    logging.info(f'Found best neighbour with parameters {best_neighbour} at a stellar {max_perf} flops/cycle')

    return best_neighbour, max_perf

def find_local_optimum(
    project_root: str, algorithm: str, form:str, vectorized: bool, input_size: int, initial_values: Tuple, find_neighbours: Callable[[Tuple, List[Tuple]], List[Tuple]]
) -> Tuple:
    """ Uses the infamous hill-climbing algorithm to find a choice of parameters with optimal performance.

        The function takes a tuple of initial values as input. Note that these values strongly influence the optimality of the found parameters.
        Further, the function takes a callable which on input of a current choice of parameters and a set of visited parameters returns a set of neighbouring parameter tuples.
        For simplicity, the callable should include the current choice in the set of neighbours if it isn't in the set of visited parameters.

        The function returns the optimal parameter tuple.
    """
    visited = list()

    curr_choice = initial_values
    curr_perf = -1.0
    while True:
        logging.info(f'Climbing around parameters {curr_choice}')
        neighbours = find_neighbours(curr_choice, visited)
        if neighbours == []:
            logging.info('No more neighbours to visit')
            break
        visited = visited + neighbours
        best_neighbour, perf = find_best_neighbour(project_root, algorithm, form, vectorized, input_size, BENCH_INPUT_DIR, neighbours)
        if perf < curr_perf:
            logging.info('No way to improve')
            break
        curr_choice = best_neighbour
        curr_perf = perf

    logging.info(f'Reached local maximum at {curr_choice}')
    return curr_choice

# TODO: Now implement the actual search steps from the paper

# ++++++++++++++++++++++++++++++++++++++++++++++++++++

def measure_fw(
    project_root,
    algorithm,
    implementation,
    compiler,
    opt_flags,
    test_input,
    input_size,
    unroll_tile_list,
):
    for ui, uj, ti, tj in unroll_tile_list:
        logging.info(f"measuring code: unroll ({ui}, {uj}), tile ({ti}, {tj})")
        measure_cmd = [
            "bash",
            f"{project_root}/team7.sh",
            "measure",
            f"{algorithm}",
            f"{implementation}-ui{ui}-uj{uj}-ti{ti}-tj{tj}",
            f"{compiler}",
            f"{opt_flags}",
            f"{test_input}",
            f"n{input_size}",
        ]

        logging.debug(" ".join(measure_cmd))
        result = subprocess.run(measure_cmd, capture_output=False, text=True)
        logging.debug(result.stdout)
        if result.returncode != 0:
            logging.error(result.stderr)
            return result.returncode

    return 0

def generate_fw(
    inpath,
    outpath,
    algorithm,
    implementation,
    compiler,
    opt_flags_raw,
    unroll_tile_list,
):
    opt_flags = opt_flags_raw.replace(" ", "_")

    generated_files = list()
    for ui, uj, ti, tj in unroll_tile_list:
        logging.info(f"generating code: unroll ({ui}, {uj}), tile ({ti}, {tj})")
        output_fname = f"{outpath}/{algorithm}_{implementation}-ui{ui}-uj{uj}-ti{ti}-tj{tj}_{compiler}_{opt_flags}.c"

        context = dict()
        context["unroll_i"] = ui
        context["unroll_j"] = uj
        context["tilesizei"] = ti
        context["tilesizej"] = tj

        if algorithm == ALGORITHM_FW:
            context["algorithm"] = "sp"
            context["datatype"] = "double"
            context["outer_op"] = "MIN"
            context["inner_op"] = "ADD"
        elif algorithm == ALGORITHM_MM:
            context["algorithm"] = "mm"
            context["datatype"] = "double"
            context["datatype"] = "double"
            context["outer_op"] = "MAX"
            context["inner_op"] = "MIN"
        elif algorithm == ALGORITHM_TC:
            context["algorithm"] = "tc"
            context["datatype"] = "char"
            context["outer_op"] = "OR"
            context["inner_op"] = "AND"
        else:
            raise Exception("Unkown algorihtm")

        with open(output_fname, mode="w", encoding="utf-8") as f:
            f.write(
                render_jinja_template(
                    f"{inpath}",
                    f"mighty-unroll-tile.py.j2",
                    **context,
                )
            )
        generated_files.append(output_fname)

    return generated_files





def validate_fw(
    project_root, algorithm, implementation, compiler, opt_flags, unroll_tile_list
):
    for ui, uj, ti, tj in unroll_tile_list:
        logging.info(f"validating code: unroll ({ui}, {uj}), tile ({ti}, {tj})")

        testcases = list()
        for n in [4, 8, 16, 30, 32]:
            if (ti == "N" or n % ti == 0) and (tj == "N" or n % tj == 0):
                testcases.append(f"n{n}")

        if len(testcases) == 0:
            # TODO
            logging.warning("Skipping validation as there are no fiting testcases")
            continue

        validate_cmd = [
            "bash",
            f"{project_root}/team7.sh",
            "validate",
            f"{algorithm}",
            f"{implementation}-ui{ui}-uj{uj}-ti{ti}-tj{tj}",
            f"{compiler}",
            f"{opt_flags}",
            f"{','.join(testcases)}",
        ]

        logging.debug(" ".join(validate_cmd))
        result = subprocess.run(
            validate_cmd,
            text=True,
            capture_output=True,
        )
        logging.debug(result.stdout)
        if result.returncode != 0:
            logging.error(result.stderr)
            return result.returncode

    return 0





def get_optimal_perf(
    data_root,
    algorithm,
    implementation,
    compiler,
    opt_flags_raw,
    test_input,
    input_size,
    unroll_tile_list,
):
    opt_flags = opt_flags_raw.replace(" ", "_")

    ui_opt = 0
    uj_opt = 0
    ti_opt = 0
    tj_opt = 0
    p_opt = 0

    for ui, uj, ti, tj in unroll_tile_list:
        logging.info(f"processing data: unroll ({ui}, {uj}), tile ({ti}, {tj})")
        data_fname = f"{data_root}/{algorithm}_{implementation}-ui{ui}-uj{uj}-ti{ti}-tj{tj}_{compiler}_{opt_flags}_{test_input}.csv"
        with open(data_fname) as f:
            reader = csv.reader(f, delimiter=",", quoting=csv.QUOTE_NONNUMERIC)
            n_list = reader.__next__()
            runs_list = reader.__next__()
            cycles_list = reader.__next__()

            n_ref = -1
            for i, n in enumerate(n_list):
                if n == input_size:
                    n_ref = i
                    break

            if n_ref == -1:
                logging.error("required column not found")
                raise Exception("failed to get optimal perf")

            c_ref = cycles_list[n_ref]
            p_ref = round((2 * n * n * n) / c_ref, 2)

            logging.debug(
                f"performance for unroll ({ui}, {uj}), tile ({ti}, {tj})= {p_ref}"
            )

            if p_ref > p_opt:
                p_opt = p_ref
                ui_opt = ui
                uj_opt = uj
                ti_opt = ti
                tj_opt = tj

    return (ui_opt, uj_opt, ti_opt, tj_opt, p_opt)


def get_best_perf(project_root, algorithm, input_size, input, unroll_tile_list):

    clean_files(project_root)

    generated_files = generate_fw(
        f"{project_root}/autotuning",
        f"{project_root}/autotuning/generated/{algorithm}",
        algorithm,
        IMPLEMENTATION,
        COMPILER,
        OPT_FLAGS,
        unroll_tile_list,
    )
    if len(generated_files) == 0:
        raise Exception("Generate files failed")

    # build all generated files
    retcode = build_files(project_root, algorithm, IMPLEMENTATION, COMPILER, OPT_FLAGS)
    if retcode != 0:
        raise Exception("Build files failed")

    # validate all built files
    retcode = validate_fw(
        project_root, algorithm, IMPLEMENTATION, COMPILER, OPT_FLAGS, unroll_tile_list
    )
    if retcode != 0:
        raise Exception("Validate files failed")

    # measure all validates files
    retcode = measure_fw(
        project_root,
        algorithm,
        IMPLEMENTATION,
        COMPILER,
        OPT_FLAGS,
        input,
        input_size,
        unroll_tile_list,
    )
    if retcode != 0:
        raise Exception("Measure files failed")

    # use measurements as feedback
    opt_ui, opt_uj, opt_ti, opt_tj, p_opt = get_optimal_perf(
        f"{project_root}/measurements/data",
        algorithm,
        IMPLEMENTATION,
        COMPILER,
        OPT_FLAGS,
        input,
        input_size,
        unroll_tile_list,
    )
    logging.info(
        f"optimal performance with:\nunrollment: ({opt_ui}, {opt_uj})\ntile: ({opt_ti}, {opt_tj})\nperformance: {p_opt}"
    )

    return (opt_ui, opt_uj, opt_ti, opt_tj, p_opt)


def unrollment_initial_guess(project_root, algorithm, is_debug_run=False):
    min_ui = 1
    max_ui = 16
    min_uj = 1
    max_uj = 32

    if is_debug_run:
        min_ui = 4
        max_ui = 6
        min_uj = 4
        max_uj = 6

    unroll_tile_list = list()
    for i in range(min_ui, max_ui + 1):
        for j in range(min_uj, max_uj + 1):
            unroll_tile_list.append((i, j, "N", "N"))

    ui, uj, ti, tj, _ = get_best_perf(
        project_root,
        algorithm,
        96 if not is_debug_run else 32,
        BENCH_INPUT if not is_debug_run else TEST_INPUT,
        unroll_tile_list,
    )

    return ui, uj


def find_local_optimum(
    project_root, algorithm, input_size, ui, uj, is_debug_run=False
):
    visited = set()
    start_rock = f"ui{ui}_uj{uj}_tiN_tjN"
    visited.add(start_rock)
    logging.info("taking first measurement for hill climbing")
    start_set = list()
    start_set.append((ui, uj, "N", "N"))
    _, _, _, _, best_perf = get_best_perf(
        project_root,
        algorithm,
        input_size,
        BENCH_INPUT,
        start_set)
    best_perf = 0
    while True:
        logging.info(f"climing hill around unrollment ({ui}, {uj})")

        curr_perf = 0
        unroll_tile_list = list()
        for i in range(ui - 2, ui + 3):
            for j in range(uj - 2, uj + 3):
                if i == ui and j == uj:
                    # skip the rock we are standing on
                    continue
                if i < 1 or j < 1:
                    # skip unrollment factors that make no sense
                    continue
                if (i > 1 and i % 2 != 0) or (j > 1 and j % 2 != 0):
                    # skip odd unrolling factors greater than 1
                    continue
                unroll_tile_list.append((i, j, "N", "N"))

                # dont climb the same rock twice
                rock = f"ui{i}_uj{j}_tiN_tjN"
                if rock in visited:
                    continue

                visited.add(rock)

        if len(unroll_tile_list) == 0:
            logging.info(f"all visited:\n{visited}")
            break

        next_ui, next_uj, next_ti, next_tj, curr_perf = get_best_perf(
            project_root,
            algorithm,
            input_size if not is_debug_run else 48,
            BENCH_INPUT,
            unroll_tile_list,
        )
        if best_perf > curr_perf or (next_ui == ui and next_uj == uj):
            logging.info(f"reached local maximum with ({ui}, {uj}) ({best_perf}flops)")
            break
        ui = next_ui
        uj = next_uj
        best_perf = curr_perf

    logging.info(f"reached top ({ui}, {uj})")
    return ui, uj


def factors_of(n):
    factors = [1]
    for i in range(2, int(math.sqrt(n)) + 1):
        if n % i == 0:
            factors.extend([i, n // i])
    factors.append(n)
    return sorted(list(set(factors)))


def tile_l2_hill_climbing(
    project_root, algorithm, input_size, l2_cache_bytes, ui, uj, is_debug_run=False
):
    input_size = input_size if not is_debug_run else 48

    factors = factors_of(input_size)
    logging.info(f"factors: {factors}")

    t2 = min(factors, key=lambda x: abs(x - int(math.sqrt(l2_cache_bytes))))

    if is_debug_run:
        t2 = 12

    if t2 == 0:
        raise Exception("heuristic makes no sense")

    visited = set()
    start_rock = f"ui{ui}_uj{uj}_ti{t2}_tj{t2}"
    visited.add(start_rock)
    logging.info("taking first measurement for hill climbing")
    start_set = list()
    start_set.append((ui, uj, t2, t2))
    _, _, _, _, best_perf = get_best_perf(
        project_root,
        algorithm,
        input_size,
        BENCH_INPUT,
        start_set)
    while True:
        logging.info(f"climing hill around unrollement ({ui}, {uj}), tile ({t2})")
        unroll_tile_list = list()
        for i in range(ui - 2, ui + 3):
            for j in range(uj - 1, uj + 3):
                if i == ui and j == uj:
                    # skip the rock we are standing on
                    continue
                if i < 1 or j < 1:
                    # skip unrollment factors that make no sense
                    continue
                if (i > 1 and i % 2 != 0) or (j > 1 and j % 2 != 0):
                    # skip odd unrolling factors greater than 1
                    continue

                fi = factors.index(t2)
                for t in [
                    factors[i] for i in [max(0, fi - 1), min(fi + 1, len(factors) - 1)]
                ]:
                    if t < 1:
                        # skip unrollment factors that make no sense
                        logging.debug(
                            "skipping rock: unrollment factor would make no sense"
                        )
                        continue

                    if i > t or j > t:
                        # tile size should ALWAYS be larger than the unrolling factor
                        logging.debug(
                            "skipping rock: tile size should be larger than unrolling factor"
                        )
                        continue

                    if t > input_size:
                        # tile size cannot be large than the testcase size
                        logging.debug(
                            "skipping rock: tile size cannot be larger than the testcase"
                        )
                        continue

                    # dont climb the same rock twice
                    rock = f"ui{i}_uj{j}_ti{t}_tj{t}"
                    if rock in visited:
                        continue

                    visited.add(rock)

                    # we use only squared tiles here
                    unroll_tile_list.append((i, j, t, t))

        if len(unroll_tile_list) == 0:
            logging.info(f"all visited:\n{visited}")
            break

        next_ui, next_uj, next_ti, next_tj, curr_perf = get_best_perf(
            project_root,
            algorithm,
            input_size,
            BENCH_INPUT,
            unroll_tile_list,
        )
        if best_perf > curr_perf or (next_ui == ui and next_uj == uj and next_ti == t2):
            break
        ui = next_ui
        uj = next_uj
        t2 = next_ti
        best_perf = curr_perf

    logging.info(f"reached top with  unrollment ({ui}, {uj}), tile ({t2}, {t2})")
    return ui, uj


def generate_optimal_source(project_root, input_size, l2_cache_bytes, algorithm):

    # find initial guess of unrolling parameters:
    # TODO: (Ui,Uj) for FWI with N = 64
    # TODO: (Ui',Uj',Uk') for FWIabc with N = 64

    # exhaustive search with test input
    #initial_ui, initial_uj = unrollment_initial_guess(
    #    project_root, algorithm, is_debug_run=debug
    #)

    # optimize FWI

    # optimize FWT

    ## hill climbing search with bench input
    initial_ui = 8
    initial_uj = 1
    refined_ui, refined_uj = find_local_optimum(
        project_root, algorithm, input_size, initial_ui, initial_uj, is_debug_run=DEBUG
    )

    #refined_ui = 10
    #refined_uj = 1
    tile_l2_hill_climbing(
        project_root,
        algorithm,
        input_size,
        l2_cache_bytes,
        refined_ui,
        refined_uj,
        is_debug_run=DEBUG,
    )


if __name__ == "__main__":
    args = parser.parse_args()
    generate_optimal_source(
        args.project_root,
        args.input_size,
        args.l2_cache,
        args.algorithm,
    )
