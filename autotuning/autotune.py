import argparse
from functools import reduce
import math
from os import path
from typing import List, Tuple, Callable
import jinja2
import subprocess
import logging
import csv

DEBUG = True
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
# parser.add_argument("--min-n", help="minimum input size", type=int, required=True)
# parser.add_argument("--max-n", help="maximum input size", type=int, required=True)
parser.add_argument(
    "-vec",
    "--vectorized",
    help="generated code should use vector instructions",
    action="store_true",
)

COMPILER = "clang"
C_FLAGS_SCALAR = "-O3 -fno-unroll-loops -fno-slp-vectorize"
C_FLAGS_VECTOR = "-O3 -march=native -ffast-math"
BENCH_INPUT_DIR = "test-inputs"
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

def generate_main(template_dir: str, output_dir: str, algorithm: str) -> str:
    """ Generates the main.c source required to run the generated implementations with the neutral element
        and datatype specified.
    """
    logging.info(f"generating main.c for {algorithm}")

    template_file = 'fw-main.py.j2'
    output_fname = f"{output_dir}/../main.c"

    context = dict()
    if algorithm == ALGORITHM_FW:
        context["algorithm"] = "sp"
        context["datatype"] = "double"
        context["neutral_element"] = 'INFINITY'
    elif algorithm == ALGORITHM_MM:
        context["algorithm"] = "mm"
        context["datatype"] = "double"
        context["neutral_element"] = 0.0
    elif algorithm == ALGORITHM_TC:
        context["algorithm"] = "tc"
        context["datatype"] = "char"
        context["neutral_element"] = 0

    with open(output_fname, mode="w", encoding="utf-8") as f:
            f.write(
                render_jinja_template(
                    f"{template_dir}",
                    template_file,
                    **context,
                )
            )

def generate_fw(template_dir: str, output_dir: str, algorithm: str, form: str, vectorized: bool, compiler: str, c_flags: str, parameters: Tuple[int, int]) -> str:
    """ Generates a FW source file and returns the implementation and more specific (parametrized) name.

        Form needs to be one of 'FWI', 'FWIabc' or 'FWT', requiring parameters
        (Ui,Uj), (Ui',Uj',Uk') or (L1,Ui,Uj,Ui',Uj',Uk'), respectively.
    """

    logging.info(f"generating source: {form} with {parameters}")

    context = dict()
    context['vector'] = vectorized
    if form == 'FWI':
        template_file = 'fw-unroll-bitwise.py.j2' if algorithm == ALGORITHM_TC else 'fw-unroll.py.j2'
        (ui, uj) = parameters
        implementation = 'vector-unroll' if vectorized else 'unroll'
        param_implementation = f'{implementation}-{ui}-{uj}'
        output_fname = f"{output_dir}/{algorithm}_{param_implementation}_{compiler}_{c_flags.replace(' ', '_')}.c"
        context['unroll_i'] = ui
        context['unroll_j'] = uj
    elif form == 'FWIabc':
        template_file = 'fw-unroll-abc-bitwise.py.j2' if algorithm == ALGORITHM_TC else 'fw-unroll-abc.py.j2'
        (uii, ujj, ukk) = parameters
        # HACK: Same filename as FWI file. Shouldn't matter because we only use this to find an initial value
        implementation = 'vector-unroll' if vectorized else 'unroll'
        param_implementation = f'{implementation}-{uii}-{ujj}-{ukk}'
        output_fname = f"{output_dir}/{algorithm}_{param_implementation}_{compiler}_{c_flags.replace(' ', '_')}.c"
        context['unroll_ii'] = uii
        context['unroll_jj'] = ujj
        context['unroll_kk'] = ukk
    elif form == 'FWT':
        template_file = 'fw-tile-bitwise.py.j2' if algorithm == ALGORITHM_TC else 'fw-tile.py.j2'
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

    return implementation, param_implementation, output_fname

def build_files(project_root: str, algorithm: str, implementation: str, compiler: str, c_flags: str) -> int:
    """ builds source file(s) as specified by the given parameters """
    build_cmd = [
        "make",
        "-C",
        f"{project_root}",
        f"build-{algorithm}-c-{implementation}-{compiler}",
        "-e",
        f"CFLAGS_DOCKER={c_flags}",
    ]

    logging.debug(" ".join(build_cmd))
    result = subprocess.run(build_cmd, stdout=subprocess.DEVNULL)
    return result.returncode

def validate_fw(
    project_root: str, algorithm: str, p_impl: str, n_factors: List[int], compiler: str, c_flags: str
) -> int:
    logging.info(f"validating code for: {p_impl}")

    testcases = list()
    for n in [4, 8, 16, 30, 32, 512]:
        if len(testcases) < 3 and reduce(lambda x, n_factor: x & (n % n_factor == 0), [True] + n_factors):
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

def measure_fw(
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
    result = subprocess.run(measure_cmd, stdout=subprocess.DEVNULL, capture_output=False, text=True)

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
    """ reads the performance of a specified implementation from a data csv file and returns it in flops/cycle """

    logging.info(f"processing performance data for: {p_impl}")
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
        impl, param_impl, _ = generate_fw(path.join(project_root, TEMPLATE_DIR), path.join(project_root, SOURCE_DIR), algorithm, form, vectorized, COMPILER, c_flags, parameters)
        param_impl_list.append((parameters, param_impl))

    # build source -'gg' for generic
    retcode = build_files(project_root, 'gg', impl, COMPILER, c_flags)
    if retcode != 0:
        raise Exception(f"Building {impl} failed")

    max_perf = -1

    for params, p_impl in param_impl_list:
        if VALIDATING and form != 'FWIabc':
            # validate correctness of neighbour
            retcode = validate_fw(project_root, algorithm, p_impl, list(params), COMPILER, c_flags)
            if retcode != 0:
                raise Exception(f"Validating {p_impl} failed")

        # measure performance
        retcode = measure_fw(project_root, algorithm, p_impl, COMPILER, c_flags, test_input_dir, input_size)
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
    project_root: str, algorithm: str, form: str, vectorized: bool, input_size: int, initial_values: Tuple, find_neighbours: Callable[[Tuple, List[Tuple]], List[Tuple]]
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
        clean_files(project_root) # save build time by always cleaning up
        logging.info(f'===== Climbing around parameters {curr_choice} =====')
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

def find_initial_guess(project_root: str, algorithm: str, vectorized: bool) -> Tuple[Tuple[int, int], Tuple[int, int, int]]:
    """ Find best (Ui,Uj) for FWI and (Ui',Uj',Uk') for FWIabc for N = 64 as an initial guess for unrolling parameters """
    def exhaustive_FWI_search(curr_params: Tuple[int, int], visited: List[Tuple[int, int]]) -> List[Tuple[int, int]]:
        if visited != []:
            return []
        else:
            return [(2**i,2**j) for i in range(5) for j in (range(2,6) if vectorized else range(6))]

    def exhaustive_FWIabc_search(curr_params: Tuple[int, int, int], visited: List[Tuple[int, int, int]]) -> List[Tuple[int, int, int]]:
        if visited == []:
            return [(2**i,2**j,1) for i in range(5) for j in (range(2,7) if vectorized else range(7))]
        elif any([k != 1 for (_,_,k) in visited]):
            return []
        else:
            (uii, ujj, _) = curr_params
            return [(uii,ujj,2**k) for k in range(6)]

    fwi_guess = find_local_optimum(project_root, algorithm, 'FWI', vectorized, 64, (0,0), exhaustive_FWI_search)
    logging.info(f'found initial guess (Ui,Uj) for FWI: {fwi_guess}')
    fwiabc_guess = find_local_optimum(project_root, algorithm, 'FWIabc', vectorized, 64, (0,0,0), exhaustive_FWIabc_search)
    logging.info(f"found initial guess (Ui',Uj',Uk') for FWIabc: {fwiabc_guess}")
    return fwi_guess, fwiabc_guess

def optimize_fwi(project_root: str, algorithm: str, vectorized: bool, initial_guess: Tuple[int, int], input_size: int, factors: List[int]) -> Tuple[int, int]:
    """ Further optimize an initial guess for FWI """
    def find_neighbours(curr_params: Tuple[int, int], visited: List[Tuple[int, int]]) -> List[Tuple[int, int]]:
        # use factors of n as possible unrollment factors
        (ui,uj) = curr_params
        if visited == []:
            neighbours = [curr_params]
        else:
            neighbours = []

        uj_factors = [f for f in factors if f % 4 == 0] if vectorized else factors

        ui_ind = factors.index(ui)
        uj_ind = uj_factors.index(uj)

        if ui_ind > 0 and (factors[ui_ind - 1], uj) not in visited:
            neighbours.append((factors[ui_ind - 1], uj))
        if ui_ind < len(factors) - 1 and (factors[ui_ind + 1], uj) not in visited:
            neighbours.append((factors[ui_ind + 1], uj))

        if uj_ind > 0 and (ui, uj_factors[uj_ind - 1]) not in visited:
            neighbours.append((ui, uj_factors[uj_ind - 1]))
        if uj_ind < len(uj_factors) - 1 and (ui, uj_factors[uj_ind + 1]) not in visited:
            neighbours.append((ui, uj_factors[uj_ind + 1]))

        return neighbours

    return find_local_optimum(project_root, algorithm, 'FWI', vectorized, input_size, initial_guess, find_neighbours)

def optimize_fwt(project_root: str, algorithm: str, vectorized: bool, initial_guess: Tuple[int, int], input_size: int, factors: List[int]) -> Tuple[int, int]:
    """ Further optimize an initial guess for FWI """
    def find_neighbours(curr_params: Tuple[int, int, int, int, int, int], visited: List[Tuple[int, int, int, int, int, int]]) -> List[Tuple[int, int, int, int, int, int]]:
        # use factors of n as possible unrollment factors
        (l1, _, _, _, _, _) = curr_params
        if visited == []:
            neighbours = [curr_params]
        else:
            neighbours = []

        for i, p in enumerate(curr_params):

            # if column unrollment and vectorized, multiple of 4
            p_factors = [f for f in factors if f % 4 == 0] if (vectorized and (i==2 or i==4)) else factors
            p_ind = p_factors.index(p)

            if (
                p_ind > 0 # neighbour exists
                and curr_params[:i] + tuple([p_factors[p_ind - 1]]) + curr_params[i + 1:] not in visited # neighbour not visited
                and ((p_factors[p_ind - 1] <= l1 and l1 % p_factors[p_ind - 1] == 0) if i!= 0 else reduce(
                    lambda x, u_factor: x & (u_factor <= p_factors[p_ind - 1]) & (p_factors[p_ind - 1] % u_factor == 0), tuple([True]) + curr_params[1:]
                )) # ensure new parameters still divide l1
            ):
                neighbours.append(curr_params[:i] + tuple([p_factors[p_ind - 1]]) + curr_params[i + 1:])

            if (
                p_ind < len(p_factors) - 1 # neighbour exists
                and curr_params[:i] + tuple([p_factors[p_ind + 1]]) + curr_params[i + 1:] not in visited # neighbour not visited
                and ((p_factors[p_ind + 1] <= l1 and l1 % p_factors[p_ind + 1] == 0) if i!= 0 else reduce(
                    lambda x, u_factor: x & (u_factor <= p_factors[p_ind + 1]) & (p_factors[p_ind + 1] % u_factor == 0), tuple([True]) + curr_params[1:]
                )) # ensure new parameters still divide l1
            ):
                neighbours.append(curr_params[:i] + tuple([p_factors[p_ind + 1]]) + curr_params[i + 1:])

        return neighbours



    return find_local_optimum(project_root, algorithm, 'FWT', vectorized, input_size, initial_guess, find_neighbours)

def factors_of(n):
    """ utility function to compute all divisors of n """
    factors = [1]
    for i in range(2, int(math.sqrt(n)) + 1):
        if n % i == 0:
            factors.extend([i, n // i])
    factors.append(n)
    return sorted(list(set(factors)))

def clean_files(project_root):
    """ utility function to clean all build and auto-generated files """
    clean_cmd = [
        "bash",
        f"{project_root}/team7.sh",
        "clean",
    ]

    logging.debug(" ".join(clean_cmd))
    result = subprocess.run(clean_cmd, stdout=subprocess.DEVNULL)
    return result.returncode


def tune_em_all(project_root: str, algorithm: str, vectorized: bool, input_size: int, l2_cache_bytes: int):
    generate_main(path.join(project_root, TEMPLATE_DIR), path.join(project_root, SOURCE_DIR), algorithm)

    (fwi_guess, fwiabc_guess) = ((4,4),(4,4,64))
    # (fwi_guess, fwiabc_guess) = find_initial_guess(project_root, algorithm, vectorized)
    logging.info('Done with step 1\n')

    factors = factors_of(input_size)
    fwi_optimal = optimize_fwi(project_root, algorithm, vectorized, fwi_guess, input_size, factors)
    logging.info('Done with step 2\n')

    l1_guess = min(
        # limit set of factors to those that are divisible by the unrollment factors
        [f for f in factors if reduce(lambda x, u_factor: x & (f % u_factor == 0), tuple([True]) + fwi_guess + fwiabc_guess)],
        key=lambda x: abs(x - int(math.sqrt(l2_cache_bytes/8)))
    )

    fwt_optimal = optimize_fwt(project_root, algorithm, vectorized, tuple([l1_guess]) + fwi_guess + fwiabc_guess, input_size, factors)
    logging.info('Done with step 3\n')

    clean_files(project_root)
    _, _, outpath_fwi = generate_fw(path.join(project_root, TEMPLATE_DIR), path.join(project_root, SOURCE_DIR), algorithm, 'FWI', vectorized, COMPILER, C_FLAGS_VECTOR if vectorized else C_FLAGS_SCALAR, fwi_optimal)
    _, _, outpath_fwt = generate_fw(path.join(project_root, TEMPLATE_DIR), path.join(project_root, SOURCE_DIR), algorithm, 'FWT', vectorized, COMPILER, C_FLAGS_VECTOR if vectorized else C_FLAGS_SCALAR, fwt_optimal)

    logging.info(f'''

    Optimal parameters for N = {input_size}:
        - FWI (Ui,Uj): {fwi_optimal}
        - FWT (L1, Ui, Uj, Ui', Uj'. Uk'): {fwt_optimal}

    Source files were generated accordingly and stored at:
        - {outpath_fwi}
        - {outpath_fwt}
    respectively.

    NOTE: Don't rerun this script unless you want to overwrite the generated files!
    ''')

if __name__ == "__main__":
    args = parser.parse_args()
    tune_em_all(args.project_root, args.algorithm, args.vectorized, args.input_size, args.l2_cache)