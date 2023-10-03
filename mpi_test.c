#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<mpi.h>

void print_array(double *arr[], int arr_size) {
    for (int j = 0; j < arr_size; j++) {
        if (j < arr_size - 1) {
            printf("%d, ", arr[j]);
        } else {
            printf("%d\n", arr[j]);
        }
    }
}

int main( int argc, char* argv[] ) {
    int arr_size;

    // Arg input handling 
    if (argc > 2) {
        printf("Too many arguments.\n");
        return 0;
    } else if (argc == 1) {
        arr_size = 2; // default to 2
    } else {
        arr_size = atoi(argv[1]);
    }

    int comm_sz;
    int my_rank;
    MPI_Init(NULL, NULL); // Initialize MPI
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz); // Get comm size
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); // Get Process rank

    double *arrays[comm_sz][arr_size];
    for(int i = 0; i < arr_size; i++) { arrays[my_rank][i] = i; }

    if (my_rank == 0) {
        for (int i = 1; i < comm_sz; i++) {
            MPI_Status status;
            double *recv[arr_size];
            MPI_Recv(&recv, arr_size, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &status );
            for (int k = 0; )
            printf("Process %d array: ", i);
            print_array(recv, arr_size);
        }
    } else 
    if (my_rank > 0) {
        print_array(arrays[my_rank], arr_size);
        MPI_Send(&arrays[my_rank][0], arr_size, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD); // Send a 1 to main process to indicate that it has populated the array
    }

    MPI_Finalize(); // Finalize MPI

    return 0;
}