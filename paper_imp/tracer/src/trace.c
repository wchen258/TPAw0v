#include "trace.h"
#include "etm.h"
#include "xtime_l.h"
#include "xil_cache.h"

extern uint32_t buffer_pointer;
extern uint32_t cur_word_index;
extern volatile uint32_t *etr_buffer;
extern void check_stop_condition(void);
extern volatile uint8_t running;
extern uint32_t g_real_time;
extern uint32_t n_slack_ct;
extern uint32_t p_slack_ct;
extern uint32_t n_times[MS_LOG_SIZE];
extern uint32_t tail_times[MS_LOG_SIZE];


static address_reg_t address_regs[3];
static XTime milestone_timings[MS_LOG_SIZE];
static uint64_t addresses[MS_LOG_SIZE];
static uint32_t cur_address = 0;
extern uint32_t resumes[256];
extern uint32_t pauses[256];
extern uint32_t resume_pt;
extern uint32_t pause_pt;
extern milestone_relay relay;
extern uint32_t rounds;

static uint32_t unexpected_ms_hit = 0;
static uint32_t timer_overflow_counter = 0;
static uint32_t num_unknown_header = 0;
static int32_t last_observed = -1;
uint8_t in_range = 0;
uint64_t prev_hit_addr = 0xffffffff;

//static void init_address_regs(void) {
//	uint8_t i;
//	for (i = 0; i < 3; ++i) {
//		address_regs[i].address = 0x0;
//		address_regs[i].is = 0x0;
//	}
//}

void trace_reset() {
	print("Trace register reset.\n\r");
	unexpected_ms_hit = 0;
	timer_overflow_counter = 0;
	num_unknown_header = 0;
	last_observed = -1;
	in_range = 0;
	prev_hit_addr = 0xffffffff;
	cur_address = 0;
}

static void update_address_regs(uint64_t address, uint8_t is) {

	address_regs[2] = address_regs[1];
	address_regs[1] = address_regs[0];
	address_regs[0].address = address;
	address_regs[0].is = is;
}

//static void handle_atom(uint8_t status) {
//	if (status) {
//		// report("E atom");
//	} else {
//		// report("N atom");
//	}
//	// report_atom(status);
//}

static void handle_address(uint64_t address, uint8_t is) {
	update_address_regs(address, is);
}


void report_results(void) {
	int i;
	for (i = 0; i < MS_LOG_SIZE; ++i) {
		milestones[i] = 0x0;
	}
	for (i = 0; i < cur_address; ++i) {
		milestones[i] = milestone_timings[i];
	}
	Xil_DCacheFlushRange(milestones, sizeof(uint32_t) * MS_LOG_SIZE);
	print("Results #ms,#ms+1,elapse(us)\n\r");
	print("LOG BEGIN\n\r");
	for (i = 1; i < current_timestamp; ++i) {
		if (milestone_timings[i] == 0) {
			xil_printf("Milestone %d was probably skipped\n\r", i);
		}
		xil_printf("%d,%d,%d,%d,%d,%d\n\r",
				i - 1,
				i,
				((milestone_timings[i] - milestone_timings[i - 1])) / COUNTS_PER_USECOND,
				(milestone_timings[i]-milestone_timings[0])/ COUNTS_PER_USECOND,
				n_times[i],
				tail_times[i]);

	}
	for (i = 0; i < cur_address; ++i) {
		xil_printf("#%d,0x%x\n\r", i, addresses[i]);
	}
	print("LOG END\n\r");
	Xil_DCacheFlush();
	print("REG BEGIN\n\r");
	for(i=0; i<pause_pt; i++){
		xil_printf("pause,%d\n\r",pauses[i]);
	}
	for(i=0;i<resume_pt; i++){
		xil_printf("resume,%d\n\r",resumes[i]);
	}
	print("REG END\n\r");
	xil_printf("negative slack ct    : %d\n\r", n_slack_ct);
	xil_printf("num timer overflew   : %d\n\r", timer_overflow_counter);
	xil_printf("unexpected milestones: %d\n\r", unexpected_ms_hit);
	xil_printf("num unknown header   : %d\n\r", num_unknown_header);
	check_stop_condition();

}




static void inc_buffer_pointer(uint32_t size) {
	uint32_t i;

	for (i = 0; i < size; ++i) {
		buffer_pointer = (buffer_pointer + 1) % (ETR_BUFFER_SIZE);

		if (buffer_pointer % 4 == 0) {
			uint32_t prev_word_pointer = (ETR_BUFFER_SIZE/4) - 1;
			if (buffer_pointer / 4 > 0)
				prev_word_pointer = (buffer_pointer / 4) - 1;

			etr_buffer[prev_word_pointer] = 0xdeadbeef; // set back to deadbeef can achieve circular buffer
			last_observed = prev_word_pointer;
		}

		if (buffer_pointer == 0)
			rounds++;
	}
}

static uint32_t read_data(uint8_t* buffer, uint32_t bytes) {
	uint32_t read;

	for (read = 0; read < bytes; ++read) {
		while (etr_buffer[buffer_pointer / 4] == 0xdeadbeef && running) {
			etr_man_flush();
		}

		if (running == 0) {
			report_results();
		}

		buffer[read] = (etr_buffer[buffer_pointer / 4] >> ((buffer_pointer % 4) * 8)) & 0xFF;
		inc_buffer_pointer(1);
	}

	return read;
}

void handle_exception(void) {
	uint8_t payload;

	read_data(&payload, 1);

	if ((payload >> 7) == 1) {
		read_data(&payload, 1);
	}
}

void handle_shortaddress(uint8_t header) {
	uint8_t payload;
	read_data(&payload, 1);
	if ((payload >> 7) == 1) {
		read_data(&payload, 1);
	}
}

void handle_longaddress(uint8_t header) {
	uint8_t payload[8];
	uint8_t is;
	uint64_t address = address_regs[0].address;

	switch (header) {
		case 0b10011010:
		// report("Long Address 32 IS0 packet");
		read_data(payload, 4);
		is = 0;
		address = address & ~((uint64_t)0xffffffff);
		address = address | (((uint64_t)payload[0] & 0x7f) << 2) |
		(((uint64_t)payload[1] & 0x7f) << 9) |
		(((uint64_t)payload[2]) << 16) | (((uint64_t)payload[3]) << 24);
		break;
		case 0b10011011:
		// report("Long Address 32 IS1 packet");
		read_data(payload, 4);
		is = 1;
		address = address & ~((uint64_t)0xffffffff);
		address = address | (((uint64_t)payload[0] & 0x7f) << 1) |
		(((uint64_t)payload[1]) << 8) | (((uint64_t)payload[2]) << 16) |
		(((uint64_t)payload[3]) << 24);
		break;
		case 0b10011101:
		// report("Long Address 64 IS0 packet");
		read_data(payload, 8);
		is = 0;
		address = 0;
		address = address | (((uint64_t)payload[0] & 0x7f) << 2) |
		(((uint64_t)payload[1] & 0x7f) << 9) |
		(((uint64_t)payload[2]) << 16) | (((uint64_t)payload[3]) << 24) |
		(((uint64_t)payload[4]) << 32) | (((uint64_t)payload[5]) << 40) |
		(((uint64_t)payload[6]) << 48) | (((uint64_t)payload[7]) << 56);

		// now the newly hit ms is calculated as address, check if its the same as previous one, if so, it's in a loop
		int i=0;
		if (in_range) {
			for(i = 0; i < relay.n_valid; i++) {
				if (address >= relay.address[i] - 4 && address <= relay.address[i] + 4) {
					register_timing();
					addresses[cur_address++] = address;
					update_graph_milestone(address);
					in_range = 0;
					break;
				}
			}

			if (in_range) {
				unexpected_ms_hit ++;
				in_range = 0;
			}
		}

		break;
		case 0b10011110:
		// report("Long Address 64 IS1 packet");
		read_data(payload, 8);
		is = 1;
		address = 0;
		address = address | (((uint64_t)payload[0] & 0x7f) << 1) |
		(((uint64_t)payload[1]) << 8) | (((uint64_t)payload[2]) << 16) |
		(((uint64_t)payload[3]) << 24) | (((uint64_t)payload[4]) << 32) |
		(((uint64_t)payload[5]) << 40) | (((uint64_t)payload[6]) << 48) |
		(((uint64_t)payload[7]) << 56);
		break;
		default:
		// report("UNDEFINED handle_longaddress header");
		xil_printf("!UNDEFINED address case, 0x%x\n!", header);
		break;
	}
	handle_address(address, is);
}

void handle_context(uint8_t header) {
	uint8_t context_info,
	vmid; // On our system (Cortex-A53), vmid is only one byte
	uint32_t contextid;

	if ((header & 0x1) == 0) {
		// report("Context packet, no payload");
	} else {
		// report("Context packet with payload");
		read_data(&context_info, 1);
		// report("payload ctl: 0x%x", context_info);
		if (((context_info >> 6) & 1) == 1) {
			read_data(&vmid, 1);
			// report("vmid: %d", vmid);
		}
		if ((context_info >> 7) == 1) {
			read_data((uint8_t *)&contextid, 4);
			// report("contextid: %d", contextid);
		}
	}
}

void handle_addrwithcontext(uint8_t header) {
	// report("Address with context");

	switch (header) {
		case 0b10000010:
		handle_longaddress(0b10011010);
		handle_context(0b10000001);
		break;
		case 0b10000011:
		handle_longaddress(0b10011011);
		handle_context(0b10000001);
		break;
		case 0b10000101:
		handle_longaddress(0b10011101);
		handle_context(0b10000001);
		break;
		case 0b10000110:
		handle_longaddress(0b10011110);
		handle_context(0b10000001);
		break;
		default:
		// report("UNDEFINED handle_addrwithcontext header");
		xil_printf("!UNDEFINED address w context header, 0x%x\n!", header);
		break;
	}
}

static inline void register_timing() {
	XTime_GetTime(&milestone_timings[current_timestamp]);
	if (current_timestamp > 0) {
		if (milestone_timings[current_timestamp] < milestone_timings[current_timestamp - 1]) {
			milestone_timings[current_timestamp] = milestone_timings[current_timestamp] + (0xFFFFFFFF - milestone_timings[current_timestamp - 1]);
			timer_overflow_counter++;
		}
		g_real_time += (milestone_timings[current_timestamp] - milestone_timings[current_timestamp-1]) / COUNTS_PER_USECOND ;
	}
	current_timestamp ++ ;
}

//static uint8_t dbg_buf[128] = {0};
//static uint32_t dbg_buf_pt = 0;

void trace_loop(void) {
	uint8_t header;
	while (running==1) {
		read_data(&header, 1);
//		dbg_buf[dbg_buf_pt++] = header;
//		dbg_buf_pt %= 128;
		switch (header) {
			case Async:
			inc_buffer_pointer(11);
			break;

			/*
			 // address ignorance version
			 case AddrWithContext2:
			 case AddrWithContext3:
			 buffer_pointer += 14;
			 break;
			 case AddrWithContext0:
			 case AddrWithContext1:
			 buffer_pointer += 10;
			 break;
			 */

			// address aware ver. more latency
			case AddrWithContext0:
			case AddrWithContext1:
			case AddrWithContext2:
			case AddrWithContext3:
			handle_addrwithcontext(header);
			break;

			case 0b10011101:// Long Address with 8B payload
			case 0b10011110:
			inc_buffer_pointer(8);
			break;
			case 0b10011010:// Long Address with 4B payload
			case 0b10011011:
			inc_buffer_pointer(4);
			break;
			case TraceInfo:
			inc_buffer_pointer(2);
			break;
			case ShortAddr0:
			case ShortAddr1:
			handle_shortaddress(header);
			break;
			case Atom10:
			case Atom11:
			break;
			case TraceOn:
			in_range = 1;
			//register_milestone();
			break;
			case Exce:
			handle_exception();
			break;
			case ExceReturn:
			break;
			case 0b01111000:// event 0
			//register_milestone();
			break;
			case 0b01110100:// event 1
			//register_milestone();
			break;
			case 0b01110010:// event 2
			//register_milestone();
			break;
			case 0b01110001:// event 3
			//register_milestone();
			break;
			default:
//			xil_printf("!UNDEFINED HEADER int:%d hex:%x\n\r", header,header);
//			xil_printf("buf pt %d\n\r", buffer_pointer);
//
//			int i;
//			for(i=0; i<dbg_buf_pt; i++) {
//				xil_printf("%u\n\r", dbg_buf[i]);
//			}
//			while(1);
			num_unknown_header++;
			break;
		}
		//cur_word_index = buffer_pointer / 4;
	}
}
















