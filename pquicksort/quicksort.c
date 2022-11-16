#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define NO_VALUE -9999999
#define DEFAULT_PATH_TO_FILE "array.txt"

int print_array(int array[], int size)
{
    int i;
    for (i = 0; i < size; ++i)
    {
        printf("%d  ", array[i]);
    }
    printf("\n");
    return 0;
}

void get_smaller_greater(int array[],
                         int array_size,
                         int smaller[],
                         int *p_smaller_size,
                         int greater[],
                         int *p_greater_size,
                         int pivot)
{
    int smaller_size = 0, greater_size = 0;

    int i;
    for (i = 0; i < array_size; i++)
    {
        if (array[i] == NO_VALUE)
        {
            continue;
        }
        if (array[i] < pivot)
        {
            smaller[smaller_size] = array[i];
            smaller_size++;
        }
        else
        {
            greater[greater_size] = array[i];
            greater_size++;
        }
    }

    *p_smaller_size = smaller_size;
    *p_greater_size = greater_size;
}

int combine_arrays(int arr1[], int arr1_size, int arr2[], int arr2_size, int arr_result[])
{
    int arr_index = 0, i;

    for (i = 0; i < arr1_size; i++)
    {
        arr_result[arr_index] = arr1[i];
        arr_index++;
    }

    for (i = 0; i < arr2_size; i++)
    {
        arr_result[arr_index] = arr2[i];
        arr_index++;
    }
    return 0;
}

int sort_array(int unsorted_array[], int array_size, int sorted_array[])
{
    int *smaller = malloc(sizeof(int) * array_size);
    int *greater = malloc(sizeof(int) * array_size);
    int smaller_size, greater_size;
    int pivot;

    pivot = unsorted_array[array_size - 1];

    get_smaller_greater(unsorted_array, array_size - 1, smaller, &smaller_size, greater, &greater_size, pivot);

    if (smaller_size > 1)
    {
        sort_array(smaller, smaller_size, smaller);
    }

    if (greater_size > 1)
    {
        sort_array(greater, greater_size, greater);
    }

    combine_arrays(smaller, smaller_size, &pivot, 1, unsorted_array);
    combine_arrays(unsorted_array, smaller_size + 1, greater, greater_size, sorted_array);
    return 0;
}

void read_file(char *path_to_file, int **pointer_to_array, int *array_size)
{
    FILE *file = fopen(path_to_file, "r");

    int i = 0;
    int num;
    int tmp_array_updated = 0;
    int *tmp_array;

    int *array = malloc(sizeof(int));

    while (fscanf(file, "%d", &num) > 0)
    {
        tmp_array = (int *)realloc(array, sizeof(int) * (i + 1));
        if (tmp_array != NULL)
        {
            array = tmp_array;
        }
        array[i] = num;
        i++;
    }
    *array_size = i;
    *pointer_to_array = array;

    fclose(file);
}

int main(int argc, char **argv)
{
    /*
    Executed by command:

    mpirun -n <number_of_processes> ./quicksort <path_to_file> <show_arrays>

    - <path_to_file>: path to array file. defaults to 'array.txt'
    - <show_arrays>: optional integer that indicates if unsorted and sorted arrays
                    are shown after execution. If 0 (false) arrays are not printed,
                    otherwise they are. Defaults to 1 (true).
    */

    MPI_Init(&argc, &argv);

    int array_size;
    int show_arrays;
    char *path_to_file;
    int **pointer_to_array = malloc(sizeof(int));

    // Reads command line arguments
    if (argc == 2)
    {
        path_to_file = argv[1];
        show_arrays = 0;
    }
    else if (argc == 3)
    {
        path_to_file = argv[1];
        show_arrays = atoi(argv[2]);
    }
    else
    {
        path_to_file = DEFAULT_PATH_TO_FILE;
    }

    read_file(path_to_file, pointer_to_array, &array_size);

    // Sorts array
    int *array = *pointer_to_array;
    int *sorted_array = malloc(sizeof(int) * array_size);

    double start_time = MPI_Wtime();
    sort_array(array, array_size, sorted_array);
    double end_time = MPI_Wtime();

    if (show_arrays)
    {
        printf("\narray: ");
        print_array(array, array_size);
        printf("\nsorted array: ");
        print_array(array, array_size);
    }
    printf("\nrun time: %f\n", end_time - start_time);

    MPI_Finalize();
    return 0;
}
