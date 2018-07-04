
#include <papi.h>
#include <stdio.h> 
#include <stdlib.h>



static int presetEvents[150];
static int nativeEvents[150];
static int init();

int main()
{

    init(); 
    

    
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



}









