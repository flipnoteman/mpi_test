//By Ryan Benton

//Libraries that are needed
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <math.h>

//Decided we won't have rows in a file greater than 1000
//characters
#define MAXSTRINGSIZE 1000

//given two different 1-D double arrays (pointA, pointB),
//calculate the distance between them.
//pointA is one array
//pointB is another
//numFeat is the number of attributes/dimensions/columns/features
//NOTE: the distance being calculated is the Manhattan distance,
//      aka L-1 norm and/or the city block distance 
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

//For each point in the test data, find the closest point in the 
//training data.
//Vars -- Input
//   trainData -- 2D array. Each row is a point
//   testData -- 2D array.  Each row is a point
//   numTrain -- the number of rows in trainData
//   numTest -- the number of rows is testData
//   numFeat -- the number of columns/features/attributes in a
//              point
//Vars -- Output
//   theMatch -- a list of size numTest.  Each entry i contains
//               the id (index) of the point in the trainingData
//               that is the closest to the ith entry in the
//               testData
//   theDist -- a list of size numTest.  Each entry i contains the
//              distance of the ith testData point to the closest
//              trainingData (which is stored in the ith entry in
//              theMatach
int getClosest(double **trainData, double **testData, int numTrain, int numTest, int numFeat, int *theMatch, double *theDist)
{
  //iterators
  int i, j;

  //local pointers.  Used to reference specific training and tes
  // points.  I initalize to NULL out of habit
  double *trainPoint = NULL;
  double *testPoint = NULL;

  //stores the distance computed between two points (one training
  //and one test
  double result = 0;

  //for each point in the testData
  for (i = 0; i < numTest; i++)
  {
      //Just to simplify the logic for me, I use testPoint to
      //reference the ith point in testData
      testPoint = testData[i];

      //for each point in numTrain
      for (j = 0; j < numTrain; j++)
      {
          //Just to simplify the logic for me, I use testPoint to
          //reference the jth point in trainData
          trainPoint = trainData[j];
       
          //find the distance the current testPoint and trainPoint
          result = calcDist(testPoint, trainPoint, numFeat);

          //if theMatch[i] contains -1, it means the current match
          //is the closest (we hadn't compared anything yet).
          //If theDist[i] > result, it means the current
          //trainPoint is closer to the ith testPoint than the
          //previous match.
          //If either case is true, we need to update the closest
          //match information
          if ((theMatch[i] == -1) || (theDist[i] > result))
          {
             //set theMatch for ith test point to list the jth 
             //trainPoint and update theDist[i] entry
             theMatch[i] = j;
             theDist[i] = result;
          }
          //habit -- trainPoint is done, so set it to NULL
          //Note: the "point" being reference by trainPoint is
          //still in the trainingData
          trainPoint = NULL;
      }

      //habit -- testPoint is done, so set it to NULL
      //Note: the "point" being reference by testPoint is
      //still in the testData
      testPoint = NULL;
  }

  //just to match the header, which requires an int return
  //hold over from an earlier version of this code.
  return 1;
}

//Initialize theMatch and theList arrays
//Input Var
//    numInst -- the number of instances (rows) to be created
//               in each array
//Output Var
//    theMatch -- a double pointer that will be used to store
//                the soon to be create 1-D int array
//    theDist  -- a double pointer that will be used to store
//                the soon to be create 1-D double array
void initMatchAndDist(int numInst, int **theMatch, double **theDist)
{
   // i is an iterator
   int i;

   // myMatch and myDist will be used to create and init the new
   //arrays
   int *myMatch = NULL;
   double *myDist = NULL;

   //create the two arrays and put them into the local myMatch and 
   //myDist
   myMatch = (int *) malloc((numInst) * sizeof(int));
   myDist = (double *) malloc((numInst) * sizeof(double));

   //Now, let's set the default values
   for (i = 0; i < numInst; i++)
   {
      //make myMatch list as -1 and myDist as 0
      myMatch[i] = -1;
      myDist[i] = 0.00;
   }

   //now, I want to make sure the newly created and initialize 
   //arrays are assigned to the output vars.  Note, since theMatch 
   //theDist is a pointer to a point, I want to make sure theMatch 
   //pointer is storing the pointer of myMatch (and same for 
   //theDist and myDist).  Hence, the double pointer is   
   //dereferenced to a single pointer.

   //Why a double pointer in the first place?  If I had sent in 
   //the single pointer, it would be a "copy" of the original 
   //single.  Which means if I reasigned the value of the single 
   //pointer copy, the update would be lost when I returned it.

   *theMatch = myMatch;
   *theDist = myDist;

   //end the functions
   return;
}

//create a 2-D array and initialize it
//Input Var
//    numInst -- the number of rows 
//    numFeat -- the number of columns/features/attributes for
//            each point/row
//Output Var
//    data -- a triple pointer.  Used to store the create 2-D
//            array
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

//Free up a 2D data array
//Input Var
//    numInst -- the number of rows 
//    numFeat -- the number of columns/features/attributes for
//            each point/row
//Input/Output Var
//    data -- a triple pointer.  Used to store the 2-D array
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

//Load data from a file into memory
//Input Var
//    fileName -- char arrary containing the input file
//Output Var
//    data -- a triple pointer.  Used to store the 2-D array that
//         will be created
//    numInst -- pointer to an integer that will be used to store 
//         the number of rows that were read
//    numFeat -- pointer to an integer that will be used to store
//         the number of columns/features/attributes for
//         each point/row
int loadData(double ***data, int *numInst, int *numFeat, char *fileName)
{
   //Init fp (the file object), line (used to read in a line from 
   //a file), linesize (num of characters in the line), and 
   //charRead.  Line, linesize and charRead are never used.
   //Why?  I was overcomplicating things.  And just never removed 
   //it

   FILE *fp = NULL;
   char *line = NULL;
   size_t linesize = 0;
   ssize_t charRead = 0;

   //Open the file. If it can't be opened/found, print an error
   //and return with a -1
   fp = fopen(fileName, "r");
   if (fp == NULL)
   {
      printf("Could not open %s.\n", fileName);
      return -1;
   }

   //Once open, get the first line -- which contains the number of 
   //rows and the number of features (columns) per row
   fscanf(fp, "%d %d", numInst, numFeat);

   //Now, using the numInst and numFeat, create the 2D matrix used 
   //to store the data
   initDataArrays(data, *numInst, *numFeat);

   //More variables. i and j are used as iterators, count is left
   //over from a previous iteration, and anInnt and aDouble are
   //used to read in data from the file

   int i, j, count, anInt;
   count = 0;
   double aDouble = 0.0;

   //for each row (obtained from numInst)
   for (i = 0; i < *numInst; i++)
   {
       //read in the row ID
       //The row ID is the same as its position in the file 
       //Really, I use it to help see if the right match is made
       //down the road.
       fscanf(fp, "%d,", &anInt);

       //Once I have the row ID, read in (numFeat - 1) doubles and 
       //assign to the data.  Since it is a triple, the index is
       //[0][i][j].  0 for the first location in the triple
       //pointer, [i] for the row, and [j]for the feature.
       for (j = 0; j < ((*numFeat) - 1); j++)
       {
           fscanf(fp, "%lf,", &aDouble);
           data[0][i][j] = aDouble;
       }
       //Then read in the last double (numFeat) and assign it
       //Why don't I just read in numFeat doubles in the loop
       //instead?  I was debugging to find a problem -- found it
       //and fixed it, but this is a holdover from the debugging
       //process.
       fscanf(fp, "%lf", &aDouble);
       data[0][i][j] = aDouble;
   }

   //Once I have read in the data, close the file
   fclose(fp);

   //Return 1 to say all is well
   return 1;
}

//Rolled my own Int verification -- hold over from another effort
//Not used
bool isAnInt(char *aString)
{
  int i, start;
  start = 0;
  if (aString[0] == '-')
     start = 1;

  for (i = start; i < strlen(aString); i++)
      if (!(isdigit(aString[i])))
         return false;
  return true;
}

//Main function
int main(int argc, char **argv)
{
   //Check number of args -- commandline args should include
   //the training and test files
   if (argc < 3)
   {
      printf("Too few arguements.\n");
      printf("Should be: %s trainFileName testFileName\n", argv[0]);
      return -1;
   }

   //these are used to calculate time.
   time_t start, end;
   double total_time;
   time(&start);

   // an iterator
   int i;

   //The number of features and the number of elements in the training and test data
   int numFeat, numTrain, numTest, tempFeat;

   //Init the ints to 0
   numFeat = numTrain = numTest = tempFeat = 0;
  
   double **trainData = NULL;
   double **testData = NULL;
 
   //need closted match array and the distance of closest match
   int *theMatch = NULL;
   double *theDist = NULL;

   //The strings to hold the file names
   char trainFileName[MAXSTRINGSIZE];
   char testFileName[MAXSTRINGSIZE];

   //clear out the strings of random noise
   memset(trainFileName, '\0', sizeof(trainFileName));
   memset(testFileName, '\0', sizeof(testFileName));
 
   //copy the file names -- no error checking
   strcpy(trainFileName, argv[1]);
   strcpy(testFileName, argv[2]);

   //Read in the training set and the test set
   loadData(&trainData, &numTrain, &numFeat, trainFileName);
   loadData(&testData, &numTest, &tempFeat, testFileName);

   //Done with the initial data sending.  Time for local cals
   //init theMatch and theDist
   initMatchAndDist(numTest, &theMatch, &theDist);

   //find the closest matches for the testData
   getClosest(trainData, testData, numTrain, numTest, numFeat, theMatch, theDist);

   //Print results 
   for (i = 0; i < numTest; i++)
   {
      printf("Test %d is closest to %d, dist: %lf\n", i, theMatch[i], theDist[i]);
   }

   time(&end);
   total_time = end - start;
   printf("The time needed was %.8lf\n", total_time);

   //Then, clean up.
   freeDataArrays(&trainData, numTrain, numFeat);
   freeDataArrays(&testData, numTest, numFeat);
   free(theMatch);
   free(theDist);
   return 0;
}
