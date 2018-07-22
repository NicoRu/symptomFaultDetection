
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
   double stat = 0; 
  
    init();

    if(PAPI_library_init(PAPI_VER_CURRENT) < PAPI_OK) {
    printf("Fehler bei PAPI Initalisierung");
    }


      for (i = 0;i < presetSize; i++ ) {

              PAPI_event_code_to_name(presetEvents[i], eventCodeStr);
            printf("----------------------------------------------------------------------------------------------\n");
            printf("                                     Measuring %s: \n", eventCodeStr);
            printf("----------------------------------------------------------------------------------------------\n");

        for (j = 0; j < MULT; j++) {

            //set up PAPI EventSet
        PAPI_create_eventset(&EventSet);


        //Measuring counter
            PAPI_add_event(EventSet, presetEvents[i]);
             
             
            printf("-------------------------------------%d. run (%s)-------------------------------------\n", j+1, eventCodeStr);
             


            PAPI_start(EventSet);
           int status = system("./run");
            PAPI_stop(EventSet, values);
        

            printf("-------------------------------------%d. run: %lld %s----------------------------------\n", j+1, values[0], eventCodeStr);

            stat += values[0];
            if(values[0] == 0) {

              
                            printf("-------------------------------------Skip %s (value 0)-----------------------------------\n", eventCodeStr);
                stat = 0; 
                j = MULT; 
            }

            //Resetting papi eventset for next perfomance counter
                PAPI_cleanup_eventset(EventSet);
                PAPI_destroy_eventset(&EventSet);
                EventSet = PAPI_NULL;
                memset(values, '\0', sizeof(values));



            }



            printf("----------------------------------------------------------------------------------------------\n");
            printf("                                Average value of %s: %f \n", eventCodeStr, stat/(MULT));
            printf("----------------------------------------------------------------------------------------------\n");


            stat = 0; 

        
        }




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










