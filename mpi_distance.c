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

#define MAXSTRINGSIZE 1000

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

/// Using mpi i was able to go through each pair that was sent using scatter, and each process will calculate the closest point by iterating through all of them on the one training point
int getClosest(double **trainData, double *testData, int numTrain, int numTest, int numFeat, int sendCnts, int *theMatch, double *theDist, int rank)
{

    int i, j;

    double *trainPoint = NULL;
    double *testPoint = NULL;

    int match = -1;
    double dist = 0;

    double result = 0;

    int matchInd = rank;
    for (int i = 0; i < sendCnts; i += numFeat){

        double *testPoint = &testData[i];

        for (int k = 0; k < numTrain; k++){
            
            result = calcDist(testPoint, trainData[k], numFeat);

            if ((match == -1) || (dist > result))
            {
             //set theMatch for ith test point to list the jth 
             //trainPoint and update theDist[i] entry
             match = k;
             dist = result;
            }
        }

        theMatch[matchInd] = match;
        theDist[matchInd] = dist;

        matchInd++;
    }


    if (rank > 0) {
        MPI_Send(&match, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Send(&dist, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }

    return 0;
}

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

   //return 1 -- I do this in case I ever want to implement error
   //codes
   return 1;
}

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

int main(int argc, char *argv[]) {
    int comm_sz;
    int rank;

    if (argc < 3)
    {
        printf("Too few arguements.\n");
        printf("Should be: %s trainFileName testFileName\n", argv[0]);
        return -1;
    }

    time_t start, end;
    double total_time;
    time(&start);
    
    int i;
    int numFeat, numTrain, numTest, tempFeat;
    numFeat = numTrain = numTest = tempFeat = 0;

    double **trainData = NULL;
    double **testData = NULL;

    int **theMatch = NULL;
    double *theDist = NULL;

    
    MPI_Init( NULL , NULL);
    MPI_Comm_size( MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank( MPI_COMM_WORLD, &rank);

    char trainFileName[MAXSTRINGSIZE];
    char testFileName[MAXSTRINGSIZE];
    
    memset(trainFileName, '\0', sizeof(trainFileName));
    memset(testFileName, '\0', sizeof(testFileName));

    strcpy(trainFileName, argv[1]);
    strcpy(testFileName, argv[2]);

    loadData(&trainData, &numTrain, &numFeat, trainFileName);
    loadData(&testData, &numTest, &tempFeat, testFileName);
    

    initMatchAndDist(numTest, &theMatch, &theDist); // Initialize match and distance

    double *data = (double *)malloc(numTest * numFeat * sizeof(double)); //Wil
    int *sendcnts = malloc(sizeof(int) * comm_sz);
    int *displs = malloc(sizeof(int) * comm_sz);

    if (rank == 0) {
        int ind = 0;
        for (int i = 0; i < numTest; i++) {
            for (int j = 0; j < numFeat; j++) {
                data[ind] = testData[i][j];
                ind++;
            }
        }

        for (int i = 0; i < comm_sz; i++) {
            sendcnts[i] = numTest * numFeat / comm_sz;
            displs[i] = i * sendcnts[i];
        }
        sendcnts[comm_sz - 1] += numTest * numFeat % comm_sz;
    }


    int recvcount = numTest * numFeat / comm_sz + ((rank == comm_sz - 1) ? (numTest * numFeat % comm_sz) : 0);
    double *recvbuf = (double *)malloc(recvcount * sizeof(double));

    MPI_Scatterv(data, sendcnts, displs, MPI_DOUBLE, recvbuf, recvcount, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    getClosest(trainData, recvbuf, numTrain, numTest, numFeat, recvcount, theMatch, theDist, rank);

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Status status;

    if (rank == 0) { // here is where we wait for the values to be sent back
        int m;
        double d;
        for (int i = 1; i < comm_sz; i++) {
            MPI_Recv(&m, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &status);
            MPI_Recv(&d, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &status);

            theMatch[i] = m;
            theDist[i] = d;
        }

        for(int i = 0; i < comm_sz; i++) {
            printf("Test ID: %d, Matched-Training ID: %d, Distance: %f\n", i, theMatch[i], theDist[i]);
        }
    }

    MPI_Finalize();

    time(&end);
    if (rank == 0){
        printf("Time: %.8ld\n", end - start);
    }

    // freeDataArrays(&trainData, numTrain, numFeat);
    // freeDataArrays(&testData, numTest, numFeat);
    // free(theMatch);
    // free(theDist);
}
