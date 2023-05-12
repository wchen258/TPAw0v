#include "etm.h"

static volatile uint8_t *prog_ctrl = CS_BASE + A53_0_ETM + TRCPRGCTLR;
static volatile uint8_t *trace_status = CS_BASE + A53_0_ETM + TRCSTATR;
static volatile uint32_t *addr_cmp_7 = CS_BASE + A53_0_ETM + TRCACVR7;
static volatile uint32_t *addr_cmp_6 = CS_BASE + A53_0_ETM + TRCACVR6;
static volatile uint32_t *addr_cmp_5 = CS_BASE + A53_0_ETM + TRCACVR5;
static volatile uint32_t *addr_cmp_4 = CS_BASE + A53_0_ETM + TRCACVR4;
static volatile uint32_t *addr_cmp_3 = CS_BASE + A53_0_ETM + TRCACVR3;
static volatile uint32_t *addr_cmp_2 = CS_BASE + A53_0_ETM + TRCACVR2;
static volatile uint32_t *addr_cmp_1 = CS_BASE + A53_0_ETM + TRCACVR1;
static volatile uint32_t *addr_cmp_0 = CS_BASE + A53_0_ETM + TRCACVR0;

static volatile uint32_t * etr_ctrl = CS_BASE + TMC3 + TMCTRG;
static volatile uint32_t * etr_ffcr = CS_BASE + TMC3 + FFCR;

volatile uint32_t * milestones = 0xfffc0000;
uint32_t milestones_size = 0;
uint32_t current_milestone = 0;
uint32_t current_timestamp = 0;
uint32_t milestone_graph [MSG_BUFFER_SIZE] = {0xffffffff};
uint32_t g_nominal_time = 0;
uint32_t n_times[MS_LOG_SIZE] = {0};
uint32_t n_times_pt = 0;
uint32_t tail_times[MS_LOG_SIZE] = {0};
uint32_t tail_times_pt = 0;
uint32_t g_real_time = 0;
uint32_t n_slack_ct = 0;
uint32_t tail_violation_ct = 0;
uint32_t p_slack_ct = 0;
milestone_relay relay;
milestone_relay tmp_relay;

extern volatile uint8_t running;
extern volatile uint32_t bandwidth_control;
extern volatile float alpha;
extern volatile float beta;
extern uint32_t margin;
uint32_t resumes[256] = {0};
uint32_t pauses[256] = {0};
uint32_t resume_pt=0;
uint32_t pause_pt=0;

void etm_reset() {
	print("RPU ETM config reset.\n\r");
	milestones = 0xfffc0000;
	milestones_size = 0;
	current_milestone = 0;
	current_timestamp = 0;
	g_nominal_time = 0;
	n_times_pt = 0;
	tail_times_pt = 0;
	g_real_time = 0;
	n_slack_ct = 0;
	tail_violation_ct = 0;
	p_slack_ct = 0;
	resume_pt=0;
	pause_pt=0;

	int i;
	for(i=0; i< MSG_BUFFER_SIZE; i++){
		milestone_graph[i] = 0xffffffff;
	}
	for(i=0; i<MS_LOG_SIZE; i++){
		n_times[i] = 0;
		tail_times[i] = 0;
	}
	for(i=0; i<256; i++) {
		resumes[i] = 0;
		pauses[i] = 0;
	}
}


void etm_disable() {
	*prog_ctrl = 0x0;
	while (!(*trace_status & 0x1));
}

void etm_enable() {
	*prog_ctrl = 0x1;
	while ((*trace_status) & 0x1);
}

void etr_disable() {
	*etr_ctrl = (*etr_ctrl) & 0x0;
}

void etr_enable() {
	*etr_ctrl = (*etr_ctrl) | 0x1;
}

void etr_man_flush() {
	*etr_ffcr |= (0x1 << 6);
}

void set_addr_cmp(uint32_t addr, int num) {
	switch (num) {
	case 0:
		*addr_cmp_0 = addr;
		*addr_cmp_1 = addr;
		break;
	case 1:
		*addr_cmp_2 = addr;
		*addr_cmp_3 = addr;
		break;
	case 2:
		*addr_cmp_4 = addr;
		*addr_cmp_5 = addr;
		break;
	case 3:
		*addr_cmp_6 = addr;
		*addr_cmp_7 = addr;
		break;
	default:
		break;
	}
}

void update_graph_milestone(uint32_t address) {
	// potential bug, Trace on might emit at none traced range, so some opt should be done, like add a flag
	etm_disable();
	int i,j;
	for(i=0; i< relay.n_valid; i++) {
		if (relay.address[i] == address) {
			g_nominal_time += relay.nominal_t[i];
			n_times[n_times_pt++] = g_nominal_time;
			tail_times[tail_times_pt++] = relay.tail_t[i];
			if (g_real_time > g_nominal_time * alpha) {
				if(bandwidth_control==1) {
				pauses[pause_pt++] = n_times_pt - 1;
				bandwidth_control = 0;
				}
			} else if (g_real_time < g_nominal_time * alpha - margin) {
				if(bandwidth_control==0){
				resumes[resume_pt++] = n_times_pt - 1;
				bandwidth_control = 1;
				}
			}
//			else if (g_real_time > relay.tail_t[i] * alpha ) {
//				if(bandwidth_control==1){
//				pauses[pause_pt++] = n_times_pt - 1;
//				bandwidth_control = 0;
//				}
//			}
			break;
		}
	}
	for(j=0; j<4; j++){
		uint32_t position = (relay.offset[i]) + 2 + 2 * j;
		uint32_t val = milestone_graph[position];
		if (val != 0xffffffff) {
			uint32_t new_address = milestone_graph[val/4];
			uint32_t new_offset = val/4;
			tmp_relay.address[j] = new_address;
			tmp_relay.offset[j] = new_offset;
			tmp_relay.tail_t[j] = milestone_graph[new_offset + 1];
			tmp_relay.nominal_t[j] = milestone_graph[position + 1];
			set_addr_cmp(new_address, j);
		} else {
			break;
		}
	}
	relay.n_valid = j;
	etm_enable();

	if (!relay.n_valid) { // this means the last milestone is reached
		running = 0;
		report_results();
	}

	for(i=0;i<relay.n_valid;i++) {
		relay.address[i] = tmp_relay.address[i];
		relay.offset[i] = tmp_relay.offset[i];
		relay.nominal_t[i] = tmp_relay.nominal_t[i];
		relay.tail_t[i] = tmp_relay.tail_t[i];
	}
}
