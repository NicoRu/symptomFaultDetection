
#include <papi.h>
#include <stdio.h> 
#include<stdlib.h>

int main()
{
    PAPI_event_info_t info;
    int retval,i =0;

int nmb = 5; 
char eventCodeStr[PAPI_MAX_STR_LEN];
retval = PAPI_library_init(PAPI_VER_CURRENT);

PAPI_event_name_to_code("PAPI_L1_DCM", &nmb);
int start = nmb;

printf("Preset Events:\n");

do {
PAPI_event_code_to_name(nmb, eventCodeStr);

printf("%s\n",eventCodeStr);

nmb++;
} while (PAPI_event_code_to_name(nmb, eventCodeStr) == PAPI_OK);

printf("Total: %d Supported Preset Counter \n", nmb - start);


int eventCode = 0 | PAPI_NATIVE_MASK;
printf("Native Events:\n");

do {
   /* Translate the integer code to a string */
   if (PAPI_event_code_to_name(eventCode, eventCodeStr) == PAPI_OK)
      /* Print all the native events for this platform */
     // printf("Name: %s\nCode: %x\n", eventCodeStr, eventCode);
printf("%s\n", eventCodeStr);
} while (PAPI_enum_event(&eventCode, 0) == PAPI_OK);


    
}










