#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<mpi.h>


/// Prints the given array in a nice format
void print_array(double arr[], int arr_size) {
    for (int j = 0; j < arr_size; j++) {
        if (j < arr_size - 1) {
            printf("%f, ", arr[j]);
        } else {
            printf("%f\n", arr[j]);
        }
    }
}

/// Sequentially prints out the arrays of every process
void print_arrays(int my_rank, double array[], int comm_sz, int arr_size) {
    if (my_rank == 0) { // If master process
        printf("Process 0 array: ");
        print_array(array, arr_size);
        for (int i = 1; i < comm_sz; i++) { // go through and fill in arrays
            MPI_Status status; // status
            double recv[arr_size]; // temp buffer for receiving the arrays
            MPI_Recv(&recv, arr_size, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &status ); // Wait until it receives the array

            printf("Process %d array: ", i);
            print_array(recv, arr_size);
        }
    } else 
    if (my_rank > 0) {
        MPI_Send(array, arr_size, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD); // Send a 1 to main process to indicate that it has populated the array
    }
}

double gen_rand(double max) {
    return( max / RAND_MAX) * rand();
}

/// Main
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
    
    // Seed the random number generation using the current time and then offset it by the processes rank (or else they will all be the same)
    srand(my_rank * time(NULL));
    
    double arrays[comm_sz][arr_size]; // Hold all the arrays from all of the processes
    double *array = &arrays[my_rank];
    for(int i = 0; i < arr_size; i++) { array[i] = gen_rand(1000.0); } // Each processes will generate random numbers for their arrays

    print_arrays(my_rank, array, comm_sz, arr_size); // print all the arrays from every process

    // Calculate the local sum
    double local_sum = 0;
    for (int i = 0; i < arr_size; i++) {
        local_sum += arrays[my_rank][i];
    }

    // Reduce to calculate global sum
    double global_sum = 0;
    MPI_Reduce(&local_sum, &global_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD); // Reduce all process sums into one global sum
    //MPI_Barrier(MPI_COMM_WORLD); // Sync processes
    double global_avg = 0;
    int rand_p;
    if (my_rank == 0) {
        global_avg = global_sum / (arr_size * comm_sz); // could have used all reduce if i didn't need to do this
        printf("\nGlobal Average: %f \n", global_avg);
        srand(time(NULL)); // seed again with time
        rand_p = (rand() % (comm_sz - 1)) + 1; // generate a random number that isn't 0
        printf("The odd-ball Process is: %d.\n", rand_p);
    }

    MPI_Bcast(&global_avg, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD); // Broadcast the global average to every process
    MPI_Bcast(&rand_p, 1, MPI_INT, 0, MPI_COMM_WORLD); // Broadcast the odd-ball rank

    // If rank is oddball, then add avg to every element of array, otherwise subtract
    if (rand_p == my_rank) {
        for (int i = 0; i < arr_size; i++) {
            array[i] += global_avg;
        }
    } else {
        for (int i = 0; i < arr_size; i++) {
            array[i] -= global_avg;
        }
    }

    print_arrays(my_rank, array, comm_sz, arr_size); // print arrays from every process sequentially

    double max = 0.0;
    for (int i = 0; i < arr_size; i++) {
        if (array[i] > max) {
            max = array[i];
        }
    }

    MPI_Barrier(MPI_COMM_WORLD); // Sync processes

    double *maxes = (double *)malloc(sizeof(double) * comm_sz); // will hold the maxes from all the processes

    MPI_Allgather(&max, 1, MPI_DOUBLE, maxes, 1, MPI_DOUBLE, MPI_COMM_WORLD); // Gather all the max values from every process into an array on every process

    if (my_rank == 0) {
        printf("\nArray Maxes: ");
        for (int i = 0; i < comm_sz; i++) {
            if (i < comm_sz - 1) {
                printf("%f, ", maxes[i]);
            } else {
                printf("%f\n", maxes[i]);
            }

        }
    }

    if (rand_p == my_rank) {
        for (int i = 0; i < arr_size; i++) {
            if (my_rank == comm_sz - 1) {
                array[i] -= maxes[0];
            } else {
                array[i] -= maxes[my_rank + 1];
            }
        }
    } else {
        for (int i = 0; i < arr_size; i++) {
            if (my_rank == comm_sz - 1) {
                array[i] += maxes[0];
            } else {
                array[i] += maxes[my_rank + 1];
            }
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    print_arrays(my_rank, array, comm_sz, arr_size);

    /// Add ID to array values
    for (int i = 0; i < arr_size; i++) {
        array[i] += my_rank;
    }

    MPI_Barrier(MPI_COMM_WORLD);

    // Get the min from every process array

    double min[comm_sz];
    MPI_Reduce(&array[0], min, arr_size, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);

    double global_min = 10000000.0;
    for (int i = 0; i < comm_sz; i++) {
        if (min[i] < global_min) {
            global_min = min[i];
        }
    }

    if (my_rank == 0) {
        printf("\nMINIMUM: %f\n", global_min);
    }

    
    MPI_Finalize(); // Finalize MPI

    return 0;
}