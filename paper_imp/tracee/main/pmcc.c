#include <stdio.h>
#include <time.h>
#include <sched.h>
#include "zcu_cs.h"
#include "cs_pmu.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>


#define BUF_SIZE (2048*1024)
#define ITER 50

int buf[BUF_SIZE];

int write_phase() {
    int i;
    for(i=0; i<BUF_SIZE; i++) {
        buf[i] = i;
    }
    return 0;
}

int read_phase() {
    int i=0;
    int j;
    for(i=0; i<BUF_SIZE; i++) {
        j = buf[i];
    }
    return 0;
}

int mix_phase() {
    int i=0;
    int j=0;
    for(i=0; i<BUF_SIZE; i++) {
        j = buf[i];
        buf[i] = 10 * j;
    }
    return 0;
}

struct timespec ts;
unsigned long ms [ITER]; 
int ct=0;

static inline void mslog() {
	clock_gettime(CLOCK_REALTIME,&ts);
	ms[ct] = (ts.tv_sec * 1000000000) + ts.tv_nsec;
	ct++;
}

static inline void cclog(PMU_interface* pmu) {
	ms[ct] = pmu->cc;
	ct++; 
} 
	


int main() {

	// set process to FIFO
	struct sched_param sp = { .sched_priority = 99 };
	int retno;
	retno = sched_setscheduler(0, SCHED_FIFO, &sp);
	if(retno == -1) {
			perror("sched set failed\n");
			return 1;
	}

	// get linux time resolution
	clock_getres(CLOCK_REALTIME, &ts);
	printf("clock_rt resolution (nanosec)  %ld \n", ts.tv_nsec);

	int policy;
	policy = sched_getscheduler(0);
	switch(policy) {
		case SCHED_OTHER:
			printf("other\n");
			break;
		case SCHED_RR:
			printf("rr\n");
			break;
		case SCHED_FIFO:
			printf("fifo\n");
			break;
		default:
			printf("something else\n");
			break;
	}


	// get PMCC
	PMU_interface *pmu = (PMU_interface *) cs_register(A53_0_pmu);
	uint64_t cc = pmu->cc;
	printf("init cc %lu\n", cc);
					
    int k,i;
	for(k=0; k<ITER; k++) {
		//printf("%lu\n",pmu->cc);
		cclog(pmu);
		for(i=0; i< 1; i++) {
				write_phase();
				read_phase();
				mix_phase();
		}

	}
	FILE *fptr = fopen("response.asap.ker","a");

	for(k=0;k<ITER-1;k++){
		fprintf(fptr,"%lu\n",ms[k]);
	
	}
	fclose(fptr);

	return 0;
}

