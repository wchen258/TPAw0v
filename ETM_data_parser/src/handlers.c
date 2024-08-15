#include "handlers.h"
#include <stdlib.h> // only for exit(EXIT_FAILURE), should not be here

static basicblock_t * ctl_ptr = 0;
static uint16_t ctl_buff_size;
static uint16_t curr_block_index;

static uint8_t ctl_state = CTL_STATE_INIT;

static uint32_t address_stack[ADDRESS_STACK_SIZE];
static uint16_t address_stack_ptr;

void report(const char* format, ... ) {
	va_list args;
	va_start(args, format);
	vfprintf(stdout, format, args);
	va_end(args);
	fprintf(stdout, "\n");
	fflush(stdout);
}

static void ctl_panic(void) {
	printf("CTL PANIC, ABORTING\n");
	fprintf(stderr, "CTL PANIC\n");
	exit(EXIT_FAILURE);
}

// TODO: Should be binary search
static uint16_t find_block(uint32_t address) {
	uint16_t i;

	for (i = 1; i < ctl_buff_size - 1; ++i) {
		if ((address >= ctl_ptr[i].start_addr) && (address < ctl_ptr[i + 1].start_addr)) {
			return i;
		}
	}

	return 0;
}

static int address_stack_push(uint32_t address) {
	if (address_stack_ptr == ADDRESS_STACK_SIZE - 1)
		return -1;

	address_stack[++address_stack_ptr] = address;
	return 0;
}

static uint32_t address_stack_pop(void) {
	if (address_stack_ptr == 0)
		return 0;
	
	return address_stack[address_stack_ptr--];
}

static inline void address_stack_clear(void) {
	address_stack_ptr = 0;
}

void set_ctl_buff(void* ptr, uint16_t size) {
	ctl_ptr = (basicblock_t *) ptr;
	ctl_buff_size = size;
	curr_block_index = 0;
	address_stack_ptr = 0;
}

void report_addres(uint64_t address64, uint8_t is) {
	uint32_t address32 = (uint32_t) (address64 & 0xffffffff);
	uint32_t popped_address;
	
	if (ctl_ptr == 0)
		return;
	
	if ((address64 != (address64 & 0xffffffff)) || !(address32 >= ctl_ptr[1].start_addr && address32 < ctl_ptr[ctl_buff_size - 1].start_addr)) {	
		if (curr_block_index != 0) {
			printf("Leaving scope to 0x%lx\n\n", address64);
			curr_block_index = 0;
			//ctl_state = (ctl_state == CTL_STATE_INIT) ? CTL_STATE_INIT : CTL_STATE_OUTSCOPE;

			// lib/kernel is able to clear the call stack. So when entering the OUTSCOPE, clear the stack and set to INIT
			address_stack_clear();
			ctl_state = CTL_STATE_INIT;
		}

		return;
	}

	printf("Entering scope block at 0x%x\n", address32);

	if (ctl_state == CTL_STATE_POP_COMP) {  
		popped_address = address_stack_pop();
		if (popped_address != address32) {
			printf("Return; Popped address (0x%x) and reported address (0x%x) do not match, halting\n", popped_address, address32);
			ctl_panic();
		} else {
			printf("Return; Pop and compare: ok\n");
		}
	}

	curr_block_index = find_block(address32);

	if (curr_block_index == 0) {
		printf("Block not found, halting\n\n");
		ctl_panic();
	}

	ctl_state = (ctl_state == CTL_STATE_PUSH) ?  CTL_STATE_INSCOPE : CTL_STATE_INIT ;

	printf("Block index: %d\n\n", curr_block_index);
}

void report_atom(uint8_t atom) {
	if (ctl_ptr == 0)
		return;

	if (curr_block_index == 0)
		return;

	printf("Atom: %c\n", atom ? 'E' : 'N');
	printf("Current block %d (0x%x): r: %d, l: %d, s: %d, c: %d, offset: 0x%x\n", curr_block_index, ctl_ptr[curr_block_index].start_addr, ctl_ptr[curr_block_index].r, ctl_ptr[curr_block_index].l,
						ctl_ptr[curr_block_index].s, ctl_ptr[curr_block_index].c, ctl_ptr[curr_block_index].offset);
	if (atom == 0) {
		if (ctl_ptr[curr_block_index].c == 0) {
			printf("C bit is 0, but Atom is N, halting\n");
			ctl_panic();
		}

		curr_block_index = curr_block_index + 1;
		printf("New block index: %d\n\n", curr_block_index);
	} else {
		if (ctl_ptr[curr_block_index].r) {
			//ctl_state = CTL_STATE_POP_COMP;
			ctl_state = (ctl_state == CTL_STATE_INSCOPE  || ctl_state == CTL_STATE_PUSH) ? CTL_STATE_POP_COMP : CTL_STATE_INIT ;
		} else {
			if (ctl_ptr[curr_block_index].l) {
				printf("Pushing 0x%x\n", ctl_ptr[curr_block_index + 1].start_addr);
				ctl_state = CTL_STATE_PUSH;
				if (address_stack_push(ctl_ptr[curr_block_index + 1].start_addr) < 0) {
					printf("Address stack overflow, halting\n");
					ctl_panic();
				}
			}

			if (ctl_ptr[curr_block_index].s) {
				curr_block_index = ctl_ptr[curr_block_index].offset / 8 + 1;
				printf("Entering block at %d, address: 0x%x\n\n", curr_block_index, ctl_ptr[curr_block_index].start_addr);
			}
		}
	}
}