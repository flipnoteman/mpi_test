/*

MPI_Distance

Samuel Smith J00688966

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <stdbool.h>
#include <mpi.h>

#define MAXSTRINGSIZE 1000 // just for file stuff

double calcDist(double *pointA, double *pointB, int numFeat)
{
   //result stores the result
   //i is used to iterate
   double result = 0.0;
   int i;

   //calculate the distance
   for (i = 0; i < numFeat; i++)
       result += fabs(pointA[i] - pointB[i]);

   return result;
}

/// Using mpi i was able to go through each pair that was sent using scatter, 
///and each process will calculate the closest point by iterating through all of them on the one training point
int getClosest(double **trainData, double *testData, int numTrain, int numTest, int numFeat, int sendCnts, int *theMatch, double *theDist, int rank)
{
    double result = 0; // our result from calculating distance

    int matchInd = 0; // Used for indexing the entire set
    for (int i = 0; i < sendCnts; i += numFeat){ // iterate through assigned points

        double *testPoint = &testData[i]; // get 1 point

        for (int k = 0; k < numTrain; k++){ // iterate through the training set
            
            result = calcDist(testPoint, trainData[k], numFeat); // Calculate distance
   
            if ((theMatch[matchInd] == -1) || (theDist[matchInd] > result))
            {
             //set theMatch for ith test point to list the jth 
             //trainPoint and update theDist[i] entry
                theMatch[matchInd] = k;
                theDist[matchInd] = result;
            }
        }


        matchInd++; // increment index
    }

    return 0;
}

/// For initializing our data arrays from files
int initDataArrays(double ***data, int numInst, int numFeat)

{
   //theData -- local variable to used to create a 2D array
   //  Not stricly necessary, but helps me avoid some pointer
   //  dereferencing steps
   double **theData;

   //create the array that will contain pointers to each row
   theData = (double **) malloc((numInst) * sizeof(double *));

   //iterators
   int i, j;

   //for each row
   for (i = 0; i < (numInst); i++)
   {
       //create the row and store it in the ith slot
       theData[i] = (double *) malloc((numFeat) * sizeof(double));

       //for each feature (column) in the row, init the value
       //I did 0 to -(numFeat - 1) to help in tracing
       for (j = 0; j < (numFeat); j++)
           theData[i][j] = (double) (i * -1);
   }

   //set *data to point to theData
   *data = theData;

   return 1;
}

/// Free the data arrays from memory once we are done
void freeDataArrays(double ***data, int numInst, int numFeat)
{
   //theData and theRow are used to simplify the deferencing logic
   //Not stricly necessary, but helps cut down my mistakes
   //(slightly)
   double **theData = NULL;
   double *theRow = NULL;

   //i is an iterator
   int i;

   //set theData to point to the 2D array stored within data
   theData = *data;

   //for each row in numInst, free up the space
   for (i = 0; i < numInst; i++)
   {
       //have theRow point to the ith row
       theRow = theData[i];

       //set the ith entry (row) in theData to NULL.  I don't want
       //to accidently down the road reference something that has
       //been released.  Habit
       theData[i] = NULL; 

       //free the malloced array that theRow currently holds
       free(theRow);
       //Set theRow to NULL -- why risk doing something in "free"
       // memory
       theRow = NULL;
   } // end of for

   //By this time, every row in theData should be freed.  Now,
   //free up the array of pointers
   free(theData);

   //Since theData pointed to the freed memory, which was also
   //held by *data, set *data to NULL
   *data = NULL;

   return;
}
/// Initialize matching and distance arrays
void initMatchAndDist(int numInst, int **theMatch, double **theDist)
{
   int i;

   int *myMatch = NULL;
   double *myDist = NULL;

   myMatch = (int *) malloc((numInst) * sizeof(int));
   myDist = (double *) malloc((numInst) * sizeof(double));

   for (i = 0; i < numInst; i++)
   {
      myMatch[i] = -1;
      myDist[i] = 0.00;
   }

   *theMatch = myMatch;
   *theDist = myDist;

   return;
}
/// Load the data from files
int loadData(double ***data, int *numInst, int *numFeat, char *fileName)
{

   FILE *fp = NULL;
   char *line = NULL;

   fp = fopen(fileName, "r");
   if (fp == NULL)
   {
      printf("Could not open %s.\n", fileName);
      return -1;
   }

   fscanf(fp, "%d %d", numInst, numFeat);

   initDataArrays(data, *numInst, *numFeat);

   int i, j, count, anInt;
   count = 0;
   double aDouble = 0.0;

   for (i = 0; i < *numInst; i++)
   {

       fscanf(fp, "%d,", &anInt);

       for (j = 0; j < ((*numFeat) - 1); j++)
       {
           fscanf(fp, "%lf,", &aDouble);
           data[0][i][j] = aDouble;
       }

       fscanf(fp, "%lf", &aDouble);
       data[0][i][j] = aDouble;
   }

   fclose(fp);

   return 1;
}

int main(int argc, char *argv[]) { // main
    int comm_sz; // communicator size
    int rank; // process rank

    if (argc < 3) // check arguments
    {
        printf("Too few arguements.\n");
        printf("Should be: %s trainFileName testFileName\n", argv[0]);
        return -1;
    }

    time_t start, end; //initialize timer variables.
    double total_time; // initialize total time
    time(&start); // get the time and store it in start
    
    int numFeat, numTrain, numTest, tempFeat; // initialize the count variables
    numFeat = numTrain = numTest = tempFeat = 0; // define them

    double **trainData = NULL; // define our training testing data double pointers to null
    double **testData = NULL;

    int *theMatch = NULL; // define our match and distance arrays to null
    double *theDist = NULL;

    // Base MPI commands
    MPI_Init( NULL , NULL);
    MPI_Comm_size( MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank( MPI_COMM_WORLD, &rank);

    char trainFileName[MAXSTRINGSIZE]; // file names
    char testFileName[MAXSTRINGSIZE];
    
    memset(trainFileName, '\0', sizeof(trainFileName)); // make memory for them and initialize to \0
    memset(testFileName, '\0', sizeof(testFileName));

    strcpy(trainFileName, argv[1]); // copy arguments into our file name handles
    strcpy(testFileName, argv[2]);

    loadData(&trainData, &numTrain, &numFeat, trainFileName); // Load data from files into train and test sets
    loadData(&testData, &numTest, &tempFeat, testFileName);
    

    initMatchAndDist(numTest, &theMatch, &theDist); // Initialize match and distance

    double *data = (double *)malloc(numTest * numFeat * sizeof(double)); // for data
    int *sendcnts = calloc(comm_sz, sizeof(int)); // initialize sendcnts to an array of 0's
    int *displs = malloc(sizeof(int) * comm_sz); // initialize displacements

    if (rank == 0) { // here we will create our sendcnts and displs arrays to use MPI_Scatterv
        int m = 0;
        int ind = 0;
        // Flatten the testData array into a 1D array "data"
        for (int i = 0; i < numTest; i++) { // 
            for (int j = 0; j < numFeat; j++) {
                data[ind] = testData[i][j];
                ind++;
            }
        }

        // Initialize our sendcnts array with numbers indicating how many points are stored in each array
        for (int i = 0; i < numTest; i++){
            if (m >= comm_sz){
                m = 0;
            }

            sendcnts[m] += numFeat;

            m++;
        }
        int tot = 0;
        // Initialize the displacement array indicating the cumulative frequency for each processor
        for (int i = 0; i < comm_sz; i++) {
            displs[i] = tot;
            tot += sendcnts[i];
        }
    }

    // Broadcast the sendcnts array to all the processes
    MPI_Bcast(sendcnts, comm_sz, MPI_INT, 0, MPI_COMM_WORLD);

    int recvcount = sendcnts[rank]; // initialize our receive count
    double *recvbuf = (double *)malloc(recvcount * sizeof(double)); // initialize our receive buffer

    // Scatter all the elements asymmetrically based on the sendcnts and displs arrays
    MPI_Scatterv(data, sendcnts, displs, MPI_DOUBLE, recvbuf, recvcount, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    getClosest(trainData, recvbuf, numTrain, numTest, numFeat, recvcount, theMatch, theDist, rank); // call the get closest function to populate theMatch and theDist

    MPI_Status status;

    if (rank == 0) { // here is where we wait for the values to be sent back
        int s = 1;
        for (int i = 1; i < comm_sz; i++) {
            MPI_Recv(&theMatch[(displs[i] / numFeat)], sendcnts[i], MPI_INT, i, 0, MPI_COMM_WORLD, &status);
            MPI_Recv(&theDist[(displs[i] / numFeat)], sendcnts[i], MPI_INT, i, 0, MPI_COMM_WORLD, &status);
            s++;
        }
        //Print the results
        for(int i = 0; i < numTest; i++) {
            printf("Test ID: %d, Matched-Training ID: %d, Distance: %f\n", i, theMatch[i], theDist[i]);
        }
    } else {
            // Transfer the Matching ID and distance for each node in the testing set
            MPI_Send(theMatch, sendcnts[rank], MPI_INT, 0, 0, MPI_COMM_WORLD);
            MPI_Send(theDist, sendcnts[rank], MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize(); // FInalize MPI

    time(&end); // store the new time in end
    if (rank == 0){
        printf("Time: %.8ld\n", end - start);
    } // Get the elapsed time from the operation
    
    freeDataArrays(&trainData, numTrain, numFeat);
    freeDataArrays(&testData, numTest, numFeat);
    free(theDist);
}
