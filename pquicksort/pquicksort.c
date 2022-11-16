#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <string.h>

#define NO_VALUE -9999999
#define NO_PIVOT -9999999

#define GREATER_SIZE_TAG 1
#define GREATER_TAG 2
#define SMALLER_SIZE_TAG 3
#define SMALLER_TAG 4

#define MIN_PARTITION_SIZE 10000

#define DEFAULT_PATH_TO_FILE "array.txt"

int MY_ID, NPROC;

void print_array(int array[], int size)
{
    int i;
    for (i = 0; i < size; ++i)
    {
        printf("%d  ", array[i]);
    }
    printf("\n");
}

void combine_arrays(int arr1[], int arr1_size, int arr2[], int arr2_size, int arr_result[])
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

void send_partitions(int array[],
                     int array_size,
                     int number_of_nodes,
                     int partitions_size,
                     int master_partition[])
{
    int array_index = 0;
    int element, node;
    int *partition = malloc(sizeof(int) * partitions_size);

    for (node = 0; node < number_of_nodes; node++)
    {
        for (element = 0; element < partitions_size; element++)
        {
            if (array_index < array_size)
            {
                partition[element] = array[array_index];
                array_index++;
            }
            else
            {
                partition[element] = NO_VALUE;
            }
        }

        if (node == MY_ID)
        {
            // This is the master node.
            memcpy(master_partition, partition, partitions_size * sizeof(int));
        }
        else
        {
            // This is a worker node.
            MPI_Send(partition, partitions_size, MPI_INT, node, 0, MPI_COMM_WORLD);
        }
    }
}

void receive_greater_smaller(
    int master_smaller[],
    int *psmaller_size,
    int master_greater[],
    int *pgreater_size,
    int total_workers,
    int worker_partition_size)
{
    // Worker's smaller and greater array variables
    int worker_smaller_size;
    int worker_greater_size;
    // Master's smaller and greater array variables
    int master_greater_index = 0;
    int master_smaller_index = 0;
    // MPI variables
    MPI_Status status;

    int worker_id, worker_smaller_index, worker_greater_index, value;
    for (worker_id = 1; worker_id < total_workers; worker_id++)
    {
        // Receive smaller array from worker
        MPI_Recv(&worker_smaller_size, 1, MPI_INT, worker_id, SMALLER_SIZE_TAG, MPI_COMM_WORLD, &status);
        int *worker_smaller = malloc(sizeof(int) * worker_smaller_size);
        MPI_Recv(worker_smaller, worker_smaller_size, MPI_INT, worker_id, SMALLER_TAG, MPI_COMM_WORLD, &status);
        // Receive greater array from worker
        MPI_Recv(&worker_greater_size, 1, MPI_INT, worker_id, GREATER_SIZE_TAG, MPI_COMM_WORLD, &status);
        int *worker_greater = malloc(sizeof(int) * worker_greater_size);
        MPI_Recv(worker_greater, worker_greater_size, MPI_INT, worker_id, GREATER_TAG, MPI_COMM_WORLD, &status);

        // Append smaller elements found
        for (worker_smaller_index = 0; worker_smaller_index < worker_smaller_size; worker_smaller_index++)
        {
            value = worker_smaller[worker_smaller_index];
            if (value != NO_VALUE)
            {
                master_smaller[master_smaller_index] = value;
                master_smaller_index++;
            }
        }
        for (worker_greater_index = 0; worker_greater_index < worker_greater_size; worker_greater_index++)
        {
            value = worker_greater[worker_greater_index];
            if (value != NO_VALUE)
            {
                master_greater[master_greater_index] = value;
                master_greater_index++;
            }
        }
    }
    *psmaller_size = master_smaller_index;
    *pgreater_size = master_greater_index;
}

int get_partition_size(int array_size, int number_of_nodes)
{
    return ceil(((double)array_size) / number_of_nodes);
}

void sort_array(int array[], int array_size, int sorted_array[], int depth)
{
    int *smaller = malloc(sizeof(int) * array_size);
    int *greater = malloc(sizeof(int) * array_size);
    int smaller_size, greater_size;
    int partition_size;
    int pivot;

    partition_size = get_partition_size(array_size - 1, NPROC);

    int *master_partition = malloc(sizeof(int) * partition_size);
    int *master_smaller = malloc(sizeof(int) * partition_size);
    int *master_greater = malloc(sizeof(int) * partition_size);
    int master_smaller_size, master_greater_size;

    // Send pivot and number of elements
    pivot = array[array_size - 1];
    MPI_Bcast(&pivot, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&partition_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    // Send partitions
    send_partitions(array, array_size - 1, NPROC, partition_size, master_partition);
    // Get master's smaller and greater
    get_smaller_greater(master_partition, partition_size, master_smaller, &master_smaller_size, master_greater, &master_greater_size, pivot);
    // Receive workers' smaller and greater arrays
    receive_greater_smaller(smaller, &smaller_size, greater, &greater_size, NPROC, partition_size);
    // Append master's smaller and greater arrays
    combine_arrays(smaller, smaller_size, master_smaller, master_smaller_size, smaller);
    smaller_size += master_smaller_size;
    combine_arrays(greater, greater_size, master_greater, master_greater_size, greater);
    greater_size += master_greater_size;

    if (smaller_size > 1)
    {
        sort_array(smaller, smaller_size, smaller, depth + 1);
    }

    if (greater_size > 1)
    {
        sort_array(greater, greater_size, greater, depth + 1);
    }

    // Combine arrays:
    int *smaller_with_pivot = malloc(sizeof(int) * (smaller_size + 1));
    combine_arrays(smaller, smaller_size, &pivot, 1, smaller_with_pivot);
    combine_arrays(smaller_with_pivot, smaller_size + 1, greater, greater_size, sorted_array);

    printf("depth=%d\n", depth);

    // End workers
    if (depth == 0 && NPROC > 1)
    {
        pivot = NO_PIVOT;
        MPI_Bcast(&pivot, 1, MPI_INT, 0, MPI_COMM_WORLD);
    }
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

    mpirun -n <number_of_processes> ./pquicksort <path_to_file> <show_arrays>

    - <path_to_file>: path to array file. defaults to 'array.txt'
    - <show_arrays>: optional integer that indicates if unsorted and sorted arrays
                     are shown after execution. If 0 (false) arrays are not printed,
                     otherwise they are. Defaults to 1 (true).
    */
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &MY_ID);
    MPI_Comm_size(MPI_COMM_WORLD, &NPROC);

    if (MY_ID == 0)
    {
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
        sort_array(array, array_size, sorted_array, 0);
        double end_time = MPI_Wtime();

        // Prints array and sorted array
        if (show_arrays)
        {
            printf("array: ");
            print_array(array, array_size);
            printf("\nsorted array: ");
            print_array(sorted_array, array_size);
        }
        printf("\nrun time: %f\n", end_time - start_time);
        free(array);
    }
    else
    {
        while (1)
        {
            int pivot, partition_size, i;

            // Receives pivot (if no pivot is found, breaks loop)
            MPI_Bcast(&pivot, 1, MPI_INT, 0, MPI_COMM_WORLD);

            if (pivot == NO_PIVOT)
            {
                break;
            }

            // Receives partition size (the number of elements the node has)
            MPI_Bcast(&partition_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

            // Obtains partition values
            int *partition = malloc(sizeof(int) * partition_size);
            MPI_Recv(partition, partition_size, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            int *smaller = malloc(sizeof(int) * partition_size);
            int *greater = malloc(sizeof(int) * partition_size);
            int smaller_size, greater_size;

            get_smaller_greater(partition, partition_size, smaller, &smaller_size, greater, &greater_size, pivot);

            MPI_Send(&smaller_size, 1, MPI_INT, 0, SMALLER_SIZE_TAG, MPI_COMM_WORLD);
            MPI_Send(smaller, smaller_size, MPI_INT, 0, SMALLER_TAG, MPI_COMM_WORLD);
            MPI_Send(&greater_size, 1, MPI_INT, 0, GREATER_SIZE_TAG, MPI_COMM_WORLD);
            MPI_Send(greater, greater_size, MPI_INT, 0, GREATER_TAG, MPI_COMM_WORLD);
        }
    }
    MPI_Finalize();
    return 0;
}
