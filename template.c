
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <papi.h>
#include <stdint.h>

#define INDEX 100
#define MULT 3


static int presetEvents[150];
static int presetSize;
static int iteration; // iteration
static int nativeSize;
static int nativeEvents[150];
static int init();
static int createData();
static float matrixa[INDEX][INDEX], matrixb[INDEX][INDEX], mresult[INDEX][INDEX];
static void setUp();
static int benchmark();
static int benchmarkFI();
static void calcStat(int value[MULT], int valueFI[MULT]);
static int print = 0;



int main()
{
    int EventSet = PAPI_NULL;
    long_long values[2];
    long_long valuesFI[2];

    char eventCodeStr[PAPI_MAX_STR_LEN];
    int i, j;
    int stat[MULT];
    int statFI[MULT];
    init();
    


    if(PAPI_library_init(PAPI_VER_CURRENT) < PAPI_OK) {
    printf("Fehler bei PAPI Initalisierung");
    }


 for (i = 0;i < presetSize; i++ ) {
        for (j = 0; j < MULT; j++) {

            //set up PAPI
        PAPI_create_eventset(&EventSet);
        setUp(); 

        //Measuring counter
            PAPI_add_event(EventSet, presetEvents[i]);
            PAPI_start(EventSet);
            benchmark();
            PAPI_stop(EventSet, values);
            //measuring counter fault injection
            stat[j] = values[0];
        setUp();



            PAPI_start(EventSet);
           iteration = benchmarkFI();
            PAPI_stop(EventSet, valuesFI);

        statFI[j] = valuesFI[0];

    
    PAPI_event_code_to_name(presetEvents[i], eventCodeStr);

        if(j == 0) {

            printf("-----------------------------------------------------------------------------------\n");
            printf("                            Measuring %s: \n", eventCodeStr);
            printf("-----------------------------------------------------------------------------------\n");
            if(values[0] == 0) {
                printf("Skipped %s (value is 0) \n", eventCodeStr);
                i++;
                print = 1; 
                j = MULT;
                goto end;
            } else {        
                printf("1. run: %lld \n", values[0]);
                }
        } else {
             printf("%d. run: %lld \n", j+1, values[0]);

            }


             printf("%d. run with FI: %lld \n", j+1, valuesFI[0]);
                printf("%d iteration (should be 100) \n",iteration);


end:
    

//Resetting papi eventset for next perfomance counter
    PAPI_cleanup_eventset(EventSet);
    PAPI_destroy_eventset(&EventSet);
    EventSet = PAPI_NULL;
    memset(values, '\0', sizeof(values));
        memset(valuesFI, '\0', sizeof(valuesFI));

        }

if(print == 0) {
printf("-----------------------------------------------------------------------------------\n");
            printf("                           Stats for %s: \n", eventCodeStr);
            printf("-----------------------------------------------------------------------------------\n");
calcStat(stat, statFI);

}

print = 0;

}


    
}

static void calcStat(int value[MULT], int valueFI[MULT]) {
double avg;
double avgFI;

int i = 0; 
    for(i = 0; i < MULT; i++) {
        avg += value[i]; 
        avgFI += valueFI[i];

    }


    avg = avg / MULT;
    avgFI = avgFI /MULT;

    double cmp = avgFI/avg;
    printf("Average: %f \nAverage with FI: %f (= %f multiple of average) \n", avg, avgFI, cmp );

print = 0;

}


static int benchmarkFI() {

int retval;
int i,j,k;
int manFactor = 1; //how often manipulation will start, 1 = always, 2 every 2nd time..

  
 //Creating some random numbers for new, faulty value.
//Counter to avoid of out of bounds errors
//printf("Manipulating index i -> faulty i... \n");
 int counter = 0;
 int man = rand() % INDEX;


  for (i=0; i<INDEX; i++) {
     if(counter == INDEX) {
      return counter; 
    }
     counter++;

   for(j=0;j<INDEX;j++) {
    for(k=0;k<INDEX;k++) {
//starting to manipulate the value of i, every 2nd iteration of i

if(counter % manFactor == 0) {
man = rand() % INDEX;

  FILE *mem = fopen("/proc/self/mem", "w");
	fseek(mem, (uintptr_t) &i, SEEK_CUR);
	fwrite(&man, sizeof(man), 1, mem);
	fclose(mem);

}
          mresult[i][j]=mresult[i][j] + matrixa[i][k]*matrixb[k][j];
    }
  }
  }

 counter; 
}


static void setUp() {


        memset(mresult, '\0', sizeof(mresult));
    memset(  matrixa, '\0', sizeof(  matrixa));
    memset(matrixb, '\0', sizeof(matrixb));

    int i; 
  for ( i=0; i<INDEX*INDEX; i++ ){
    mresult[0][i] = 0.0;
  matrixa[0][i] = matrixb[0][i] = rand()*(float)1.1; }
}




static int benchmark() {
     int i,j,k;
  /* Matrix-Matrix multiply */
  for (i=0;i<INDEX;i++)
   for(j=0;j<INDEX;j++)
    for(k=0;k<INDEX;k++)
      mresult[i][j]=mresult[i][j] + matrixa[i][k]*matrixb[k][j];
}



/*Initialize PAPI Library and the supported events on the platform*/
static int init() {

int retval,i = 0;
int nmb = 0; 
char eventCodeStr[PAPI_MAX_STR_LEN];
retval = PAPI_library_init(PAPI_VER_CURRENT);
//PAPI_L1_DCM is the first preset event
PAPI_event_name_to_code("PAPI_L1_DCM", &nmb);
int start = nmb;

//printf("Preset Events:\n");

do {
if(PAPI_event_code_to_name(nmb, eventCodeStr) == PAPI_OK) {
presetEvents[i] = nmb;
//printf("%s\n",eventCodeStr);
i ++;
nmb++;
}
} while (PAPI_event_code_to_name(nmb, eventCodeStr) == PAPI_OK);

//printf("Total: %d Supported Preset Counter \n", nmb - start);
presetSize = nmb-start; 
i = 0;
int eventCode = 0 | PAPI_NATIVE_MASK;
//printf("Native Events:\n");

do {
   /* Translate the integer code to a string */
   if (PAPI_event_code_to_name(eventCode, eventCodeStr) == PAPI_OK) {       
   nativeEvents[i] = eventCode;
    i++;
   }
      /* Print all the native events for this platform */
      //  printf("%s\n", eventCodeStr);
   } while (PAPI_enum_event(&eventCode, 0) == PAPI_OK);

//printf("Total: %d Supported Native Counter \n", i);
nativeSize = i;


}


static int createData() {

/*
    // insert the date into the char array
char text[17];
time_t now = time(NULL);
struct tm *t = localtime(&now);
strftime(text, sizeof(text)-1, "%dd %mm %YYYY %HH:%MM", t);
text[16] = 0;

// concat the date to file name
char *filename;
if((filename = malloc(strlen("C:\\Temp\\filename.txt")+strlen(text)+1)) != NULL){
    filename[0] = '\0';   // ensures the memory is an empty string
    strcat(filename,"C:\\Temp\\filename");
    strcat(filename,text);
    strcat(filename,".txt");
}*/
     FILE *fp = NULL;

        fp = fopen("output" ,"a");


     


        if (fp != NULL) {

                fprintf(fp, "Change Case \n");
        } 

            fclose(fp);

        

}









