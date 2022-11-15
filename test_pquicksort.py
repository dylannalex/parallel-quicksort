import os
import sys
import random


PQUICKSORT_PATH = "pquicksort/./pquicksort"
RANDOM_FILE_BUFFER = "random_array_buffer.txt"


def generate_random_array(path_to_file, min_, max_, array_size):
    # type: (str, int, int, int) -> None
    random_array = ["%s " % random.randint(min_, max_) for _ in range(array_size)]
    file = open(path_to_file, "w")
    file.writelines(random_array)
    file.close()


def run_pquicksort(number_of_subprocesses, path_to_file):
    # type: (str, int) -> None
    args = (number_of_subprocesses, PQUICKSORT_PATH, path_to_file)
    command = "mpirun -n %s %s %s" % args
    os.system(command)


def execution_log(iteration, min_, max_, array_size):
    # type: (int, int, int, int) -> None
    """Prints array iteration, min, max and size."""
    top = "\n\n" + "-" * 100 + "\n"
    array_number = "array " + str(iteration + 1)
    min_argument = "\n -min: " + str(min_)
    max_argument = "\n -max: " + str(max_)
    array_size_argument = "\n -array size: " + str(array_size)
    bottom = "\n\n"
    print(
        top + array_number + min_argument + max_argument + array_size_argument + bottom
    )


def random_command(arguments):
    # type: (list[str]) -> None
    """
    python test_pquicksort.py random <number_of_subprocesses> <min> <max> <array_one_size> <array_two_size> ...

    - <number_of_subprocesses>: number of subprocesses used for executing mpirun.
    - <min>: the smallest number that can be generated in a random array.
    - <max>: the greatest number that can be generated in a random array.
    - <array_n_size>: total numbers in nth array.
    """
    number_of_subprocesses = int(arguments[2])
    min_ = int(arguments[3])
    max_ = int(arguments[4])
    for i, argument in enumerate(arguments[5:]):
        array_size = int(argument)
        execution_log(i, min_, max_, array_size)
        generate_random_array(RANDOM_FILE_BUFFER, min_, max_, array_size)
        run_pquicksort(number_of_subprocesses, RANDOM_FILE_BUFFER)


def custom_command(arguments):
    # type: (list[str]) -> None
    """
    python test_pquicksort.py custom <path_to_file> <number_of_subprocesses>

    - <path_to_file>: path to array file.
    - <number_of_subprocesses>: number of subprocesses used for executing mpirun.
    """
    path_to_file = arguments[2]
    number_of_subprocesses = int(arguments[3])
    run_pquicksort(number_of_subprocesses, path_to_file)


def generate_random_array_command(arguments):
    # type: (list[str]) -> None
    """
    python test_pquicksort.py generate_random_array <path_to_file> <min> <max> <array_size>

    - <path_to_file>: path to array file.
    - <min>: the smallest number that can be generated in the random array.
    - <max>: the greatest number that can be generated in the random array.
    - <array_size>: total numbers in the array.
    """
    path_to_file = arguments[2]
    min_ = int(arguments[3])
    max_ = int(arguments[4])
    array_size = int(arguments[5])
    generate_random_array(path_to_file, min_, max_, array_size)


def main():
    """
    python test_pquicksort.py <command> <args>

    - <command>: Command to run. Can either be one of the following:
        - random
        - custom
        - generate_random_array
    """
    arguments = sys.argv
    command = arguments[1]

    if command == "generate_random_array":
        generate_random_array_command(arguments)
    if command == "random":
        random_command(arguments)
    if command == "custom":
        custom_command(arguments)


if __name__ == "__main__":
    main()
