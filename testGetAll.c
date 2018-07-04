
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <papi.h>
#define INDEX 100



static int presetEvents[150];
static int presetSize;
static int nativeSize;
static int nativeEvents[150];
static int init();
static int createData();
static float matrixa[INDEX][INDEX], matrixb[INDEX][INDEX], mresult[INDEX][INDEX];
static void setUp();
static int benchmark();




int main()
{
    int EventSet = PAPI_NULL;

    init();
    
    if(PAPI_library_init(PAPI_VER_CURRENT) < PAPI_OK) {
    printf("Fehler");
    }
    
    PAPI_create_eventset(&EventSet);

    setUp(); 
    PAPI_add_event(EventSet, presetEvents[0]);
    long_long values[1];

    PAPI_start(EventSet);
         benchmark();
    PAPI_stop(EventSet, values);

 

 printf("...%lld \n", values[0]);

 

 printf("...%lld \n", values[8]);




    createData();

    
}


static void setUp() {
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

int retval,i =0;
int nmb = 0; 
char eventCodeStr[PAPI_MAX_STR_LEN];
retval = PAPI_library_init(PAPI_VER_CURRENT);
//PAPI_L1_DCM is the first preset event
PAPI_event_name_to_code("PAPI_L1_DCM", &nmb);
int start = nmb;

printf("Preset Events:\n");

do {
if(PAPI_event_code_to_name(nmb, eventCodeStr) == PAPI_OK) {
presetEvents[i] = nmb;
printf("%s\n",eventCodeStr);
i ++;
nmb++;
}
} while (PAPI_event_code_to_name(nmb, eventCodeStr) == PAPI_OK);

printf("Total: %d Supported Preset Counter \n", nmb - start);
presetSize = nmb-start; 
i = 0;
int eventCode = 0 | PAPI_NATIVE_MASK;
printf("Native Events:\n");

do {
   /* Translate the integer code to a string */
   if (PAPI_event_code_to_name(eventCode, eventCodeStr) == PAPI_OK) {       
   nativeEvents[i] = eventCode;
    i++;
   }
      /* Print all the native events for this platform */
        printf("%s\n", eventCodeStr);
   } while (PAPI_enum_event(&eventCode, 0) == PAPI_OK);

printf("Total: %d Supported Native Counter \n", i);
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









