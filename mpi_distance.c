#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <stdbool.h>
#include <mpi.h>

#define MAXSTRINGSIZE 1000;

int main(int argc, char *argv[]) {
    int comm_sz;
    int rank;
    MPI_Init( NULL , NULL);
    MPI_Comm_size( MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank( MPI_COMM_WORLD, &rank);

    

    MPI_Finalize();
}
