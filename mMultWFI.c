/*
 *Based on the example from PAPIDocs.
 This programm runs a 100x100 matrix multiplikation and measures time, rtime, ptime, flop-instructions and floating point operation.
 The second run is influenced by memory fault injection. Whereas the index of the first matrix/array is randomly changed and therefore 
 leading to false results.
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <memory.h>
#include <malloc.h>
#include "papi.h"
#include <time.h>

#define INDEX 100
#define n 4

static void test_fail(char *file, int line, char *call, int retval);
static void matrixMultiply();
static void matrixMultiplywMemManipulation();
static void reset(); 

int main(int argc, char **argv) {
clock_t begin;
clock_t end;
double timeSpent;
double timeSum;
double timeSpent1;
double timeSum1;
for (int i = 0; i < n; i++) {
  begin = clock();
    matrixMultiply();
  end = clock();
printf("___________________________________________\n");
timeSpent = (double)(end - begin)/CLOCKS_PER_SEC;  
timeSum = timeSum + timeSpent;

      begin = clock();
        matrixMultiplywMemManipulation();
      end = clock();
printf("___________________________________________\n");
timeSpent1 = (double)(end - begin)/CLOCKS_PER_SEC;  
timeSum1 = timeSum1 + timeSpent1;
}

printf("Average execution time with no faults:, %f \n", timeSum/n);
printf("Average execution time with fault:, %f \n", timeSum1/n);
}

//Trying to reset cpu cache, taking way too long
static void reset(){

  printf("Enter reset \n");
   const int size =20*1024;//*1024; // Allocate 20M. Set much larger then L2 i < 0xffff
     char *c = (char *)malloc(size);
     for (int i = 0; i < size; i++){
       for (int j = 0; j < size; j++) {
         c[j] = i*j;
         printf(". \n");
       }
     }

     printf ("\n end reset \n");

/*
 echo 3 > /proc/sys/vm/drop_caches && swapoff -a && swapon -a && printf '\n%s\n' 'Ram-cache and Swap Cleared'
OR
$ su -c "echo 3 >'/proc/sys/vm/drop_caches' && swapoff -a && swapon -a && printf '\n%s\n' 'Ram-cache and Swap Cleared'" root
*/

}
static void matrixMultiply() {

extern void dummy(void *);
  float matrixa[INDEX][INDEX], matrixb[INDEX][INDEX], mresult[INDEX][INDEX];
  float real_time, proc_time, mflops;
  long long flpins;
  int retval;
  int i,j,k;
printf("Starting execution without any faults..\n");
  /* Initialize the Matrix arrays */
  for ( i=0; i<INDEX*INDEX; i++ ){
    mresult[0][i] = 0.0;
  matrixa[0][i] = matrixb[0][i] = rand()*(float)1.1; }

  /* Setup PAPI library and begin collecting data from the counters */
  if((retval=PAPI_flops( &real_time, &proc_time, &flpins, &mflops))<PAPI_OK)
    test_fail(__FILE__, __LINE__, "PAPI_flops", retval);

  /* Matrix-Matrix multiply */
  for (i=0;i<INDEX;i++)
   for(j=0;j<INDEX;j++)
    for(k=0;k<INDEX;k++)
      mresult[i][j]=mresult[i][j] + matrixa[i][k]*matrixb[k][j];

  /* Collect the data into the variables passed in */
  if((retval=PAPI_flops( &real_time, &proc_time, &flpins, &mflops))<PAPI_OK)
    test_fail(__FILE__, __LINE__, "PAPI_flops", retval);

  printf("Execution without faults: \n Real_time:\t%f\nProc_time:\t%f\nTotal flpins:\t%lld\nMFLOPS:\t\t%f\n",
  real_time, proc_time, flpins, mflops);
  PAPI_shutdown();
}


static void matrixMultiplywMemManipulation() {

float matrixa[INDEX][INDEX], matrixb[INDEX][INDEX], mresult[INDEX][INDEX];
float real_time, proc_time, mflops;
long long flpins;
int retval;
int i,j,k;

  /* Initialize the Matrix arrays */
  for ( i=0; i<INDEX*INDEX; i++ ){
    mresult[0][i] = 0.0;
    matrixa[0][i] = matrixb[0][i] = rand()*(float)1.1; }

  /* Setup PAPI library and begin collecting data from the counters */
  if((retval=PAPI_flops( &real_time, &proc_time, &flpins, &mflops))<PAPI_OK)
    test_fail(__FILE__, __LINE__, "PAPI_flops", retval);
  
 //Creating some random numbers for new, faulty value.
//Counter to avoid of out of bounds errors
printf("Manipulating index i -> faulty i... \n");
 int counter = 0;
 int man = rand() % INDEX;
  for (i=0;i<INDEX;i++)
   for(j=0;j<INDEX;j++)
    for(k=0;k<INDEX;k++) {
//starting to manipulate the value of i
  FILE *mem = fopen("/proc/self/mem", "w");
	fseek(mem, (uintptr_t) &i, SEEK_CUR);
	fwrite(&man, sizeof(man), 1, mem);
	fclose(mem);
    man = rand() % INDEX;
    counter++;
    printf("(%d->%d) ", counter, i);
    if(counter%10 == 0) {
      printf("\n");
    }
          mresult[i][j]=mresult[i][j] + matrixa[i][k]*matrixb[k][j];
          if(counter == 100) {
      goto end; 
    }
    }

end: printf("\n");
  /* Collect the data into the variables passed in */
  if((retval=PAPI_flops( &real_time, &proc_time, &flpins, &mflops))<PAPI_OK)
    test_fail(__FILE__, __LINE__, "PAPI_flops", retval);

  printf("Fault execution: \n Real_time:\t%f\nProc_time:\t%f\nTotal flpins:\t%lld\nMFLOPS:\t\t%f\n",
  real_time, proc_time, flpins, mflops);
  PAPI_shutdown();
}


static void test_fail(char *file, int line, char *call, int retval){
    printf("%s\tFAILED\nLine # %d\n", file, line);
    if ( retval == PAPI_ESYS ) {
        char buf[128];
        memset( buf, '\0', sizeof(buf) );
        sprintf(buf, "System error in %s:", call );
        perror(buf);
    }
    else if ( retval > 0 ) {
        printf("Error calculating: %s\n", call );
    }
    else {
        printf("Error in %s: %s\n", call, PAPI_strerror(retval) );
    }
    printf("\n");
    exit(1);
}