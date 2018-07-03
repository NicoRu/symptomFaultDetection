
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <memory.h>
#include <malloc.h>
#include <papi.h>
#include <sys/time.h>

#define INDEX 300
struct timeval t1, t2;
//static void test_fail(char *file, int line, char *call, int retval);
static double mMult();
static double mMult_wFlops();
static double mMult_wFlips();
static double mMult_wCyc();
static double mMult_wIpc();
static double mMult_wEvents();
static double mMult_singleEvent();
double elapsedTime(struct timeval t1, struct timeval t2);
static void test_fail(char *file, int line, char *call, int retval);
double calcStat(double t1, double t2);
void setUp();
void reset();

float matrixa[INDEX][INDEX], matrixb[INDEX][INDEX], mresult[INDEX][INDEX];
int condBranch[7] = { PAPI_BR_CN, PAPI_BR_INS,PAPI_BR_MSP, PAPI_BR_NTK ,PAPI_BR_PRC ,PAPI_BR_TKN,PAPI_BR_UCN /*,PAPI_BRU_IDL ,PAPI_BTAC_M*/};
void allEvents(int events[], int length);

int main(int argc, char **argv) {

 
setUp();

double t1 = mMult();
reset();
printf("Execution time without any metrics: %f ms \n", mMult());
printf("__HIGH LEVEL API__ \n");
double t2 = mMult_wFlops();
reset();
printf("Execution time with measuring pTime, mFlops, flpins: %f ms \n", t2);
printf ("Overhead: %f percent \n", calcStat(mMult(),t2));

double t3 = mMult_wFlips();
reset();
printf("Execution time with measuring pTime, mFlips, flpins: %f ms \n", t3);
printf ("Overhead: %f percent \n", calcStat(mMult(),t3));

double t5 = mMult_wIpc();
reset();
printf("Execution time with measuring instruction per cycles: %f ms \n", t5);
printf ("Overhead: %f percent \n", calcStat(mMult(),t5));

printf("__LOW LEVEL API__ \n");

double t4 = mMult_wCyc();
reset();
printf("Execution time with measuring clock cycles: %f ms \n", t4);
printf ("Overhead: %f percent \n", calcStat(mMult(),t4));



int Events[2] = {PAPI_L2_DCM, PAPI_L2_DCA};
double t6 = mMult_wEvents(Events, 2);
reset();
printf("Execution time with measuring misses/accesses in L2 cache: %f ms \n", t6);
printf ("Overhead: %f percent \n", calcStat(mMult(),t6));


printf("__Conditional Branching__\n");

allEvents(condBranch,7);


printf("__Cache Requests __\n");

int cacheReq[5] = {PAPI_CA_CLN,PAPI_CA_INV,PAPI_CA_ITV,PAPI_CA_SHR,PAPI_CA_SNP};

allEvents(cacheReq,5);


printf("__Conditional Store__\n");

int conStore[3] = {PAPI_CSR_FAL,PAPI_CSR_SUC, PAPI_CSR_TOT};

allEvents(conStore,3);
printf("__Floating Point Operations__\n");
int flop[14] ={PAPI_FAD_INS,PAPI_FDV_INS,PAPI_FMA_INS,PAPI_FML_INS, PAPI_FNV_INS ,PAPI_FP_INS ,PAPI_FP_OPS ,PAPI_FP_STAL,PAPI_FPU_IDL,PAPI_FSQ_INS, PAPI_SP_OPS, PAPI_DP_OPS, PAPI_VEC_SP, PAPI_VEC_DP};

allEvents(flop,14);


printf("__Instruction Counting__\n");

int instr[9] = {PAPI_FUL_CCY, PAPI_FUL_ICY, PAPI_FXU_IDL, PAPI_HW_INT, PAPI_INT_INS, PAPI_TOT_CYC,PAPI_TOT_IIS,PAPI_TOT_INS,PAPI_VEC_INS };

allEvents(instr,9);
printf("__Cache Access__\n");
int cache[] = {PAPI_L1_DCA,PAPI_L1_DCH ,PAPI_L1_DCM ,PAPI_L1_DCR,PAPI_L1_DCW ,PAPI_L1_ICA ,PAPI_L1_ICH,PAPI_L1_ICM ,PAPI_L1_ICR ,PAPI_L1_ICW ,PAPI_L1_LDM ,PAPI_L1_STM ,PAPI_L1_TCA ,PAPI_L1_TCH ,PAPI_L1_TCM ,PAPI_L1_TCR ,PAPI_L1_TCW ,PAPI_L2_DCA ,PAPI_L2_DCH ,PAPI_L2_DCM ,PAPI_L2_DCR,PAPI_L2_DCW ,PAPI_L2_ICA ,PAPI_L2_ICH ,PAPI_L2_ICM ,PAPI_L2_ICR ,PAPI_L2_ICW ,PAPI_L2_LDM,PAPI_L2_STM ,PAPI_L2_TCA ,PAPI_L2_TCH ,PAPI_L2_TCM ,PAPI_L2_TCR ,PAPI_L2_TCW ,PAPI_L3_DCA ,PAPI_L3_DCH ,PAPI_L3_DCM ,PAPI_L3_DCR ,PAPI_L3_DCW ,PAPI_L3_ICA ,PAPI_L3_ICH ,PAPI_L3_ICM ,PAPI_L3_ICR ,PAPI_L3_ICW ,PAPI_L3_LDM ,PAPI_L3_STM ,PAPI_L3_TCA ,PAPI_L3_TCH ,PAPI_L3_TCM ,PAPI_L3_TCR ,PAPI_L3_TCW};

allEvents(cache,sizeof(cache)/sizeof(cache[0]));
 	
printf("__Date Access_\n");

int date[] = {PAPI_LD_INS 	,PAPI_LST_INS 	,PAPI_LSU_IDL 	,PAPI_MEM_RCY 	,PAPI_MEM_SCY 	,PAPI_MEM_WCY 	,PAPI_PRF_DM 	,PAPI_RES_STL ,PAPI_SR_INS ,PAPI_STL_CCY ,PAPI_STL_ICY ,PAPI_SYC_INS};


allEvents(date,sizeof(date)/sizeof(date[0]));

printf("__TLB_\n");

 int tlb[] = {PAPI_TLB_DM,PAPI_TLB_IM,PAPI_TLB_SD,PAPI_TLB_TL};


allEvents(tlb,sizeof(tlb)/sizeof(tlb[0]));


//Doing random event collection


printf("-----Measuring two parallel, random events-----\n");


int l2[] = {PAPI_L2_DCM,PAPI_L1_ICM};
double t10 = mMult_wEvents(l2,2);
printf("Execution time parallel (PAPI_L2_DCM,PAPI_L1_ICM) : %f ms \n", t10);
printf ("Overhead: %f percent \n", calcStat(mMult(),t10));
double t11 = mMult_singleEvent(PAPI_L1_ICM);
printf("Execution time single (PAPI_L1_ICM) : %f ms; Overhead to parallel %f \n", t11, calcStat(t10,t11));
double t12 = mMult_singleEvent(PAPI_L2_DCM);
printf("Execution time single (PAPI_L2_DCM) : %f ms; Overhead to parallel %f \n", t12, calcStat(t10,t12));
reset();

printf("-----Measuring three parallel, random events-----\n");


int l3[] = {PAPI_TLB_DM, PAPI_TOT_CYC, PAPI_L1_ICM};
t12 = mMult_wEvents(l3,3);
printf("Execution time parallel (PAPI_TLB_DM, PAPI_TOT_CYC, PAPI_L1_ICM}) : %f ms \n", t12);
printf ("Overhead: %f percent \n", calcStat(mMult(),t12));
t11 = mMult_singleEvent(PAPI_TLB_DM);
printf("Execution time single (PAPI_TLB_DMM) : %f ms; Overhead to parallel %f \n", t11, calcStat(t12,t11));
double t13 = mMult_singleEvent(PAPI_TOT_CYC);
printf("Execution time single (PAPI_TOT_CYC) : %f ms; Overhead to parallel %f \n", t13, calcStat(t12,t13));
reset();
int k[] = {PAPI_L1_ICM};
double t14 = mMult_wEvents(k,1);
printf("Execution time single (PAPI_L1_ICM) : %f ms; Overhead to parallel %f \n", t14, calcStat(t12,t14));

printf("-----Measuring four parallel, random events-----\n");
int l4[] = {PAPI_TLB_DM, PAPI_TOT_CYC, PAPI_L1_ICM, PAPI_L3_ICM};

t12 = mMult_wEvents(l4,5);
printf("Execution time parallel (PAPI_TLB_DM, PAPI_LD_INS, PAPI_L1_ICM,PAPI_TLB_IM}) : %f ms \n", t12);
printf ("Overhead: %f percent \n", calcStat(mMult(),t12));
t11 = mMult_singleEvent(PAPI_TLB_DM);
printf("Execution time single (PAPI_TLB_DM) : %f ms; Overhead to parallel %f \n", t11, calcStat(t12,t11));
 t13 = mMult_singleEvent(PAPI_TOT_CYC);
printf("Execution time single (PAPI_TOT_CYC) : %f ms; Overhead to parallel %f \n", t13, calcStat(t12,t13));
reset();
t14 = mMult_singleEvent(PAPI_L1_ICM);
printf("Execution time single (PAPI_L1_ICM) : %f ms; Overhead to parallel %f \n", t14, calcStat(t12,t14));
double t15 = mMult_singleEvent(PAPI_FDV_INS);
printf("Execution time single (PAPI_TLB_IM) : %f ms; Overhead to parallel %f \n", t14, calcStat(t12,t15));



  exit(0);
}





void allEvents(int events[], int length) {
char EventCodeStr[PAPI_MAX_STR_LEN];
for (int i = 0; i < length; i++) {

	PAPI_event_code_to_name(events[i], EventCodeStr );
        if(PAPI_query_event(events[i]) == PAPI_OK) {

	double t8 =  mMult_singleEvent(events[i]);
		printf("Execution time with measuring %s : %f ms \n", EventCodeStr, t8);
		printf ("Overhead: %f percent \n", calcStat(mMult(),t8));
	} else {printf("%s not supported \n", EventCodeStr);}
}



}



void setUp(){
  //Initialize the Matrix arrays 
  for (int i=0; i<INDEX*INDEX; i++ ){
    mresult[0][i] = 0.0;
    matrixa[0][i] = matrixb[0][i] = rand()*(float)1.1; }
}
void reset(){
memset(mresult, 0, sizeof(mresult));
memset(matrixa, 0, sizeof(matrixa));
memset(matrixb, 0, sizeof(matrixb));
setUp();
}

//Simple matrix multy
static double mMult() {
struct timeval t1, t2;

gettimeofday(&t1, NULL);
int i,j,k;

  for (i=0;i<INDEX;i++)
   for(j=0;j<INDEX;j++)
    for(k=0;k<INDEX;k++)
      mresult[i][j]=mresult[i][j] + matrixa[i][k]*matrixb[k][j];

gettimeofday(&t2, NULL);
reset(); 
return elapsedTime(t1,t2); }

//matrix multy with measuring clock cycles
static double mMult_wCyc() {
struct timeval t1, t2;
int i,j,k;
long long s;

gettimeofday(&t1, NULL);

s = PAPI_get_real_cyc();
  for (i=0;i<INDEX;i++)
   for(j=0;j<INDEX;j++)
    for(k=0;k<INDEX;k++)
      mresult[i][j]=mresult[i][j] + matrixa[i][k]*matrixb[k][j];
long long e = PAPI_get_real_cyc();
long long cyc = e-s;
gettimeofday(&t2, NULL);

return elapsedTime(t1,t2); }

//mMult with measuring events
static double mMult_singleEvent(int op) {

struct timeval t1, t2;

int i,j,k;
long long values[1];
unsigned int native = 0x0;
gettimeofday(&t1, NULL);
int retval, EventSet = PAPI_NULL;

retval = PAPI_library_init(PAPI_VER_CURRENT);

if (retval != PAPI_VER_CURRENT) {
  fprintf(stderr, "PAPI library init error!\n");
  exit(1);
}


if (PAPI_create_eventset(&EventSet) != PAPI_OK)
printf("Fehler1 ");

if (PAPI_add_event(EventSet, op) != PAPI_OK)
printf("fehler2");
/* Start counting */
if (PAPI_start(EventSet) != PAPI_OK)
printf("Fehler1 ");

/* Do some computation here */

if (PAPI_read(EventSet, values) != PAPI_OK)
printf("Fehler1 ");

/* Do some computation here */
  for (i=0;i<INDEX;i++)
   for(j=0;j<INDEX;j++)
    for(k=0;k<INDEX;k++)
      mresult[i][j]=mresult[i][j] + matrixa[i][k]*matrixb[k][j];

if (PAPI_stop(EventSet, values) != PAPI_OK)
printf("Fehler1 ");

PAPI_reset(EventSet);

reset();
gettimeofday(&t2, NULL);
return elapsedTime(t1,t2); 
}


static double mMult_wEvents(int Events[], int length) {

struct timeval t1, t2;
gettimeofday(&t1, NULL);
int i,j,k;

long long values[length];
unsigned int native = 0x0;

int retval, EventSet = PAPI_NULL;

/* Initialize the PAPI library */
retval = PAPI_library_init(PAPI_VER_CURRENT);

if (retval != PAPI_VER_CURRENT) {
  fprintf(stderr, "PAPI library init error!\n");
  exit(1);
}


if (PAPI_create_eventset(&EventSet) != PAPI_OK)
printf("Fehler1 ");


/* Start counting events */
if (PAPI_start_counters(Events, length) != PAPI_OK){
printf("Fehler2 %d", PAPI_start_counters(Events, length));
return -1; }

    for (i=0;i<INDEX;i++)
   for(j=0;j<INDEX;j++)
    for(k=0;k<INDEX;k++)
      mresult[i][j]=mresult[i][j] + matrixa[i][k]*matrixb[k][j];

/* Read the counters */
if (PAPI_read_counters(values, length) != PAPI_OK)
printf("Fehler3");

/* Do some computation here */
/*
 for (i=0;i<INDEX;i++)
   for(j=0;j<INDEX;j++)
    for(k=0;k<INDEX;k++)
      mresult[i][j]=mresult[i][j] + matrixa[i][k]*matrixb[k][j];
/* Stop counting events */
if (PAPI_stop_counters(values, length) != PAPI_OK)
printf("Fehler4");

PAPI_stop(EventSet, values);
PAPI_shutdown();
reset(); 


gettimeofday(&t2, NULL);


return elapsedTime(t1,t2);

}
static double mMult_wFlops() {
struct timeval t1, t2;
int i,j,k;
  float real_time, proc_time, mflops;
  long long flpins;
  int retval;

gettimeofday(&t1, NULL);
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

  /*printf("Real_time:\t%f\nProc_time:\t%f\nTotal flpins:\t%lld\nMFLOPS:\t\t%f\n",
  real_time, proc_time, flpins, mflops);*/
  //printf("%s\tPASSED\n", __FILE__);
  PAPI_shutdown();
    gettimeofday(&t2, NULL);

	return elapsedTime(t1,t2); 
}

static double mMult_wFlips() {
struct timeval t1, t2;
int i,j,k;
  float real_time, proc_time, mflips;
  long long flpins;
  int retval;

gettimeofday(&t1, NULL);
  /* Setup PAPI library and begin collecting data from the counters */
  if((retval=PAPI_flips( &real_time, &proc_time, &flpins, &mflips))<PAPI_OK)
    test_fail(__FILE__, __LINE__, "PAPI_flips", retval);

  /* Matrix-Matrix multiply */
  for (i=0;i<INDEX;i++)
   for(j=0;j<INDEX;j++)
    for(k=0;k<INDEX;k++)
      mresult[i][j]=mresult[i][j] + matrixa[i][k]*matrixb[k][j];

  /* Collect the data into the variables passed in */
  if((retval=PAPI_flips( &real_time, &proc_time, &flpins, &mflips))<PAPI_OK)
    test_fail(__FILE__, __LINE__, "PAPI_flops", retval);
/*
  printf("Real_time:\t%f\nProc_time:\t%f\nTotal flpins:\t%lld\nMFLOPS:\t\t%f\n",
  real_time, proc_time, flpins, mflips);
  //printf("%s\tPASSED\n", __FILE__);*/
  PAPI_shutdown();
    gettimeofday(&t2, NULL);

	return elapsedTime(t1,t2); 
}
static double mMult_wIpc() {
struct timeval t1, t2;
int i,j,k;
  float real_time, proc_time,ipc;
  long long ins;
  int retval;

gettimeofday(&t1, NULL);
  /* Setup PAPI library and begin collecting data from the counters */
  if((retval=PAPI_ipc( &real_time, &proc_time, &ins, &ipc))<PAPI_OK)
    test_fail(__FILE__, __LINE__, "PAPI_flips", retval);

  /* Matrix-Matrix multiply */
  for (i=0;i<INDEX;i++)
   for(j=0;j<INDEX;j++)
    for(k=0;k<INDEX;k++)
      mresult[i][j]=mresult[i][j] + matrixa[i][k]*matrixb[k][j];

  /* Collect the data into the variables passed in */
  if((retval=PAPI_ipc( &real_time, &proc_time, &ins, &ipc))<PAPI_OK)
    test_fail(__FILE__, __LINE__, "PAPI_flops", retval);

 // printf("Real_time:\t%f\nProc_time:\t%f\nTotal flpins:\t%lld\nMFLOPS:\t\t%f\n",
  //real_time, proc_time, ins, ipc);

  PAPI_shutdown();

    gettimeofday(&t2, NULL);

	return elapsedTime(t1,t2); 
}

/*Calculating the time passed between start time t1 and end time t2 in ms*/
double elapsedTime (struct timeval t1,struct timeval t2) {
double time; 
time = (t2.tv_sec - t1.tv_sec) * 1000.0;
	time += (t2.tv_usec - t1.tv_usec) / 1000.0;

return time;
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

double calcStat(double t1, double t2) {

return (t2/t1 - 1 ) * 100;

}

