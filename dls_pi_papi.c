/*
 *  Dynamic Linking System (DLS)
 * ------------------------------
 *
 *	Mario Kicherer (kicherer@kit.edu)
 *
 */

/**
 *	@file
 *	@brief PAPI sensor plugin
 *	@ingroup plugins
 */


#include <stdio.h>
#include <pthread.h>
#include <papi.h>

#define DLS_MOD_NAME "DLS-PI-PAPI"

#include "dls_pi.h"
#include "dls_il.h"

#define CHECK_RET(retval) \
	if (retval != PAPI_OK) \
		DLS_ERROR("return value %d: %s\n", retval, PAPI_strerror(retval))

#define N_EVENTS 3
static char *event_names[] = {
	"instr_count",
	"branch_count",
	"hw_int"
};

static struct dls_plugin * self;
static struct dls_sensor sensors[N_EVENTS];

pthread_key_t dls_papi_tdata;

struct pu_data {
	int event_index;
	// 	struct thread_data *tdata;
	struct dls_hwtop_pu *pu;
};

struct thread_data {
	int event_set;
// 	int event_index;
// 	char started;
	struct dls_hwtop_pu *cur_pu;
	
// 	struct pu_data *cur_pud;
	
	unsigned int n_events; // number of active events
	
	long_long values[N_EVENTS];
	
	struct thread_data *master;
	
	pthread_mutex_t slave_lock;
	struct thread_data **slaves;
	unsigned int n_slaves;
	long_long slave_values[N_EVENTS];
};

static void add_event(int event_set, char *name) {
	int ecode;
	
	DLS_DEBUG("add event %s\n", name);
	
	CHECK_RET( PAPI_event_name_to_code(name, &ecode) );
	CHECK_RET( PAPI_add_event(event_set,  ecode) );
}

static struct thread_data *get_tdata(void *pu_ptr) {
// 	struct pu_data *pud; //, *master;
// 	int i;
	struct dls_hwtop_pu *pu = (struct dls_hwtop_pu *) pu_ptr;
	
	
	struct thread_data *tdata = (struct thread_data*) pthread_getspecific(dls_papi_tdata);
	
	if (tdata)
		return tdata;
// 	DLS_MEAS_START;
	tdata = (struct thread_data*) dls_calloc(sizeof(struct thread_data));
	
	if (DLS_BF_TEST(pu->pu_group->pmodels, PM_OCL)) {
		DLS_DEVEL("ignoring OpenCL device %s\n", pu->id);
		return 0;
	}
	
// 	for (i=0;i<N_EVENTS;i++) {
// 		tdata = (struct pu_data*) dls_calloc(sizeof(struct pu_data));
		tdata->event_set = PAPI_NULL;
		
// 		PAPI_register_thread();
		
// 		if (i == 0) {
			CHECK_RET( PAPI_set_domain(PAPI_DOM_USER) );
			CHECK_RET( PAPI_create_eventset(&tdata->event_set) );
			
			if (DLS_BF_TEST(pu->pu_group->pmodels, PM_CPU)) {
				add_event(tdata->event_set, "PAPI_TOT_INS");
				add_event(tdata->event_set, "PAPI_BR_INS");
				add_event(tdata->event_set, "PAPI_HW_INT");
				tdata->n_events = 3;
			} else
			if (DLS_BF_TEST(pu->pu_group->pmodels, PM_GPU)) { // & PM_CUDA
				PAPI_assign_eventset_component(tdata->event_set, 2);
				
				if (!strcmp(pu->pu_group->id, ".GeForce_GTX_275")) {
					add_event(tdata->event_set, "cuda:::GeForce_GTX_275:domain_b:instructions");
					add_event(tdata->event_set, "cuda:::GeForce_GTX_275:domain_b:branch");
					tdata->n_events = 2;
				} 
				else if (!strcmp(pu->pu_group->id, ".GeForce_GTX_560_Ti")) {
					add_event(tdata->event_set, "cuda:::GeForce_GTX_560_Ti:domain_d:inst_executed");
					tdata->n_events = 1;
				}
			}
// 			master = tdata;
// 		} else {
// 			tdata->master = master;
// 		}
		tdata->cur_pu = pu;
		pthread_mutex_init(&tdata->slave_lock, 0);
// 		tdata->event_index = i;
		
		pthread_setspecific(dls_papi_tdata, tdata);
		
// 		pu->sensor_data[sensors[i].index] = tdata;
// 	}
// 	DLS_MEAS_STOP("papi");
	return tdata;
}

static void free_pu(struct dls_hwtop_pu * pu) {
	int i;
	
	for (i=0;i<N_EVENTS;i++) {
		struct pu_data *pud = (struct pu_data*) pu->sensor_data[sensors[i].index];
		
		if (!pud)
			return;
		
// 		if (pud->slaves)
// 			dls_free(pud->slaves);
		
		dls_free(pud);
	}
}

static void new_puc(struct dls_hwtop_pu_group * puc) {
	struct dls_hwtop_pu * pit;
	int i;
	
	for (pit=puc->pus;pit;pit = pit->next) {
// 		new_pu(pit, puc->id);
		for (i=0;i<N_EVENTS;i++) {
			struct pu_data *pud = (struct pu_data*) dls_malloc(sizeof(struct pu_data));
// 			pud->tdata = 0;
			pud->event_index = i;
			pud->pu = pit;
			pit->sensor_data[sensors[i].index] = pud;
		}
	}
}

static void free_puc(struct dls_hwtop_pu_group * puc) {
	struct dls_hwtop_pu * pit;
	for (pit=puc->pus;pit;pit = pit->next) {
		free_pu(pit);
	}
}

static char start_counter(void * data) {
	struct pu_data *pud = (struct pu_data*) data;
	struct thread_data *tdata = get_tdata(pud->pu);
	
	if (!pud || pud->event_index==0)
		CHECK_RET( PAPI_start(tdata->event_set) );
	
	return 0;
}

static char get_counter(void * data, dls_hist_value * value) {
	struct pu_data *pud = (struct pu_data*) data;
	int retval, i;
	struct thread_data *tdata = get_tdata(0);
	
	if (pud && pud->event_index > 0 && tdata->n_events <= pud->event_index)
		return 0;
	
	if (!pud || pud->event_index == 0) {
		retval = PAPI_read(tdata->event_set, tdata->values);
		
		if (retval != PAPI_OK) {
			DLS_ERROR("return value %d: %s\n", retval, PAPI_strerror(retval));
			return 0;
		}
// 		printf("val %lld\n", tdata->values[pud->event_index]);
// 		if (value)
// 			*value = tdata->values[pud->event_index];
	}
	if (value)
		*value = tdata->values[pud->event_index];
	
	if (tdata->n_slaves && value) {
// 		long_long t;
// 		*value = 0;
		*value = tdata->slave_values[pud->event_index];
		for (i=0;i<tdata->n_slaves;i++) {
// 			printf("adding %lld to %ld\n",tdata->slaves[i]->slave_values[pud->event_index], *value);
// 			printf("master %ld\n", *value);
			*value += tdata->slaves[i]->slave_values[pud->event_index];
// 			t = tdata->slaves[i]->values[pud->event_index] - tdata->slaves[i]->slave_values[pud->event_index];
// 			printf("adding %lld to %ld\n",t, *value);
// 			*value += t;
		}
	}
	
	
	return 1;
}

static char stop_counter(void * data) {
	struct pu_data *pud = (struct pu_data*) data;
	struct thread_data *tdata = get_tdata(0);
	
	if (!pud || pud->event_index==0)
		CHECK_RET( PAPI_stop(tdata->event_set, tdata->values) ) ;
	
	return 0;
}

void init_plugin(struct dls_plugin * plugin) {
	int res, i, num_components;
	
	self = plugin;
	self->type = DLS_PIT_SENSOR;
	
	DLS_MEAS_START;
	
	res = PAPI_library_init(PAPI_VER_CURRENT);
	if (res != PAPI_VER_CURRENT) {
		DLS_ERR("PAPI initialization failed due to wrong PAPI version.\n");
		return;
	}
	
	res = PAPI_thread_init(pthread_self);
	if (res != PAPI_OK) {
		DLS_DEVEL("initialization failed: %d \"%s\"\n", res, PAPI_strerror(res));
		return;
	}
	
	num_components = PAPI_num_components();
	DLS_DEBUG("Number of installed components: %d\n", num_components);
	
	const PAPI_component_info_t *info;
	for (i=0; i<num_components; i++) {
		info = PAPI_get_component_info(i);
		if (!info)
			DLS_ERROR("Error on retrieving component info %i\n", i);
		
		DLS_DEBUG("Component \"%s\" (%d) version %s\n", info->name, i, info->version);
		DLS_DEBUG("\tHardware counters: %d\n", info->num_cntrs);
		DLS_DEBUG("\tSupported preset events: %d\n", info->num_preset_events);
		DLS_DEBUG("\tSupported native events: %d\n", info->num_native_events);
	}
	
	DLS_MEAS_STOP("PAPI init");
	
	pthread_key_create(&dls_papi_tdata, 0);
	
	self->n_sensors = N_EVENTS;
}

void add_sensors(unsigned char index) {
	struct dls_sensor * m;
	int i;
	
	for (i=0;i<N_EVENTS;i++) {
// 		m = (struct dls_sensor*) dls_calloc(sizeof(struct dls_sensor));
		m = &sensors[i];
		m->id = event_names[i];
		m->plugin = self;
		m->index = index+i;
		m->start_counter = &start_counter;
		m->get_counter = &get_counter;
		m->stop_counter = &stop_counter;
		
		dls_root->sensors[index+i] = m;
	}
	
	dls_hwtop_add_pugroup_cb(&new_puc, &free_puc);
}

void release_plugin() {
	PAPI_shutdown();
}

void *dls_papi_get_master() {
// 	struct dls_call_state *cstate = dls_get_cstate(0);
// 	return cstate;
	return pthread_getspecific(dls_papi_tdata);
}

void dls_papi_start_slave(void *master_tdata) {
// 	struct dls_call_state *master = (struct dls_call_state*) master_cstate;
	struct thread_data *tdata = (struct thread_data*) pthread_getspecific(dls_papi_tdata);
	
// 	if (!tdata) {
// 		printf("init_slave\n");
// // 		tdata = (struct thread_data*) dls_calloc(sizeof(struct thread_data));
// // 		pthread_setspecific(dls_papi_tdata, tdata);
// 		
// // 		for (i=0;i<N_EVENTS;i++) {
// 			struct thread_data *mtdata;
// 			mtdata = (struct thread_data *) master->map->pus->pu->sensor_data[sensors[0].index];
// 			mtdata->n_slaves++;
// 			mtdata->slaves = (struct thread_data*) dls_realloc(mtdata->slaves, sizeof(struct thread_data*)*mtdata->n_slaves);
// 			tdata = &mtdata->slaves[mtdata->n_slaves-1];
// 			
// 			tdata->event_set = PAPI_NULL;
// // 		}
// 		
// 		pthread_setspecific(dls_papi_tdata, tdata);
// 		
// // 		CHECK_RET( PAPI_set_domain(PAPI_DOM_USER) );
// 		CHECK_RET( PAPI_create_eventset(&tdata->event_set) );
// 		
// 		// 		if (DLS_BF_TEST(pu->pu_group->pmodels, PM_CPU)) {
// 		add_event(tdata->event_set, "PAPI_TOT_INS");
// 		add_event(tdata->event_set, "PAPI_BR_INS");
// 		add_event(tdata->event_set, "PAPI_HW_INT");
// 		// 		}
// 		start_counter(tdata);
// 	}
	
	struct thread_data *mtdata = (struct thread_data *) master_tdata;
	
	
	tdata = get_tdata(mtdata->cur_pu);
	
// 	if (tdata == mtdata)
// 		return;
	
// 	if (tdata->started)
// 		return;
	
	if (tdata->master && tdata->master != mtdata) {
		DLS_ERR("TODO\n");
	}
	
	if (!tdata->master && tdata != mtdata) {
		pthread_mutex_lock(&mtdata->slave_lock);
// 		printf("add slave %d\n", mtdata->n_slaves+1);
	// 	mtdata = (struct thread_data *) master->map->pus->pu->sensor_data[sensors[0].index];
		mtdata->n_slaves++;
		mtdata->slaves = (struct thread_data**) dls_realloc(mtdata->slaves, sizeof(struct thread_data*)*mtdata->n_slaves);
		mtdata->slaves[mtdata->n_slaves-1] = tdata;
		pthread_mutex_unlock(&mtdata->slave_lock);
	// 	pthread_setspecific(dls_papi_tdata, tdata);
		tdata->master = mtdata;
	}
// 	printf("start\n");
// 	tdata->started = 1;
// 	start_counter(0);
	
	if (tdata != mtdata)
		CHECK_RET( PAPI_start(tdata->event_set) );
	get_counter(0, 0);
// 	printf("a %lld\n", tdata->slave_values[0]);
// 	memcpy(tdata->slave_values, tdata->values, sizeof(long_long)*N_EVENTS);
	int i;
	for (i=0;i<N_EVENTS;i++) {
		tdata->slave_values[i] -= tdata->values[i];
	}
	
	if (tdata != mtdata)
		memset(tdata->values, 0, sizeof(long_long)*N_EVENTS);
// 	for (i=0;i<N_EVENTS;i++) {
// 		tdata->values[i] = 
// 	}
}

void dls_papi_stop_slave(void *master_tdata) {
	int i;
	struct thread_data *tdata = (struct thread_data*) pthread_getspecific(dls_papi_tdata);
	
	struct thread_data *mtdata = (struct thread_data *) master_tdata;
	
// 	if (tdata == mtdata)
// 		return;
	
	if (tdata == mtdata)
		get_counter(0, 0);
	else
		stop_counter(0);
// 	printf("stop\n");
	for (i=0;i<N_EVENTS;i++) {
		tdata->slave_values[i] += tdata->values[i];
	}
// 	printf("b %lld\n", tdata->slave_values[0]);
}


















