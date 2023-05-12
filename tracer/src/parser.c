//#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xtime_l.h"
#include "etm.h"
#include "xil_cache.h"

volatile uint32_t etr_buffer_unused[ETR_BUFFER_SIZE] __attribute__((section(".trc_buf_zone")));
volatile uint32_t * etr_buffer = &etr_buffer_unused[0]; //(uint32_t *) 0xB0000000;
volatile uint8_t running = 0;
volatile uint32_t bandwidth_control = 1;
volatile float alpha = 1.3;
volatile float beta = 1;
volatile uint32_t t_end = 0;
uint32_t margin = 0;

extern void trace_loop(void);
extern milestone_relay relay;
extern uint32_t milestone_graph[MSG_BUFFER_SIZE];

uint32_t buffer_pointer; // pointer to individual bytes in etr_buffer
uint32_t cur_word_index = 0 ;
uint32_t rounds = 0;
static uint32_t last_word = 0xdeadbeef;
static uint32_t last_word_index = ETR_BUFFER_SIZE + 1;

void parser_reset() {
	print("Parser register reset.\n\r");
	etr_buffer = &etr_buffer_unused[0];
	running = 0;
	bandwidth_control = 1;
	alpha = 1.3;
	beta = 1;
	t_end = 0;
	margin = 0;
	cur_word_index = 0 ;
	rounds = 0;
	last_word = 0xdeadbeef;
	last_word_index = ETR_BUFFER_SIZE + 1;
}


void cache_graph(volatile uint32_t *src, uint32_t size, uint32_t *dst) {
	int i=0;
	for(i=0; i< size; i++) {
		dst[i] = src[i];
	}
}

void init_relay(milestone_relay *relay) {
	relay -> n_valid      = 1;
	relay -> address[0]   = milestone_graph[0];
	relay -> offset[0]    = 0;
	relay -> nominal_t[0] = 0;
	relay -> tail_t[0]    = 0;
}


void check_stop_condition(void) {
	if (running == 0) {
		xil_printf("Running Stopped\n\r");
		xil_printf("Buffer used: %d/%d\n\r", rounds * ETR_BUFFER_SIZE + buffer_pointer, ETR_BUFFER_SIZE);
		Xil_DCacheFlush(); // if DCache not flushed, buffer dump would not work correctly. However not guarantee to work

		while(running == 0);
		if(running==2) {
			;
		}
	}
}


//uint8_t data_available() {
//    return running;
//}

void start() {
	// Set deadbeef to buffers. deadbeef is marker for unavailable data
	unsigned int i;
    for (i = 0; i < ETR_BUFFER_SIZE; ++i) {
    	etr_buffer[i] = 0xdeadbeef;
    }
    for (i = 0; i < MSG_BUFFER_SIZE; ++i) {
    	milestones[i] = 0xffffffff;
    }
    Xil_DCacheFlush();

    xil_printf("\n\r");
    xil_printf("TPAw0v Tracer. T-Graph Circular Buffer. Stack at 0x%x\n\r", &i);
    xil_printf("Global Tightly Couple Memory addr offset: 0xffe00000\n\r");
    xil_printf("Tracer   ctl: %x, sizeof(Xtime)=%d\n\r", (uint32_t) &running, sizeof(XTime));
    xil_printf("Corunner ctl: %x\n\r", (uint32_t) &bandwidth_control);
    xil_printf("alpha    ctl: %x\n\r", (uint32_t) &alpha);
    xil_printf("beta     ctl: %x\n\r", (uint32_t) &beta);
    xil_printf("T_nom    ctl: %x\n\r", (uint32_t) &t_end);
    print("Set alpha ,beta, t_end before run application!\n\r");
    print("Can be set from host by devmem\n\r");
    usleep(20000000);
    xil_printf("Waiting for milestones to be set at 0x%x\n\r", &milestones[0]);

    while(milestones[0] != 0xdeadbeef) {
    	Xil_DCacheInvalidateRange(milestones, sizeof(uint32_t) * MSG_BUFFER_SIZE);
    }
    margin = (uint32_t) ((float) t_end * (1-beta));
    xil_printf("Read 0xdeadbeef, wait for hoster driver configuration\n\r");
    xil_printf("margin: %d\n\r", margin);
    if (margin == 0) {
    	xil_printf("margin is zero\n\r");
    } else {
    	xil_printf("margin is non-zero\n\r");
    }

    usleep(1000000);

    milestones_size = milestones[1];
    milestones = &milestones[2];

    cache_graph(milestones, milestones_size, milestone_graph);
    set_addr_cmp(milestone_graph[0], 0);  // before ETM starts, set the first address in trace range
    init_relay(&relay);

    if (milestones_size >= MSG_BUFFER_SIZE) {
    	xil_printf("milestones_size should be < %d\n\r", MSG_BUFFER_SIZE);
    }

    print("hand off to trace loop.\n\r");
    print("\n\r");

    buffer_pointer = 0;
    rounds = 0;
    last_word = 0xdeadbeef;
    last_word_index = ETR_BUFFER_SIZE + 1;
    trace_loop();
    check_stop_condition();
}


int main() {
	init_platform();

	while(1) {
		running = 1;
		start();
		while(running != 2);
		print("RPU reset request.\n\r");
		trace_reset();
		etm_reset();
		parser_reset();
	}

	cleanup_platform();
}


