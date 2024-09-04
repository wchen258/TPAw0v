#ifndef COMMON_H_
#define COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>

#define SET(x, y) ((x) |= (1 << (y)))
#define CHECK(x,y) (((x) & (1 << (y))) ? 1 : 0)
#define CLEAR(x,y) ((x) &= ~(1 << (y)))

int write_mem(unsigned long physical_address, uint32_t data);
void pin_to_core(uint8_t id);
void linux_disable_cpuidle(void);
void spawn_child(void (*func)());
void poller();
uint32_t wrmem(char* bin_name, unsigned long addr);

/*  Initialize the memory at [buf_addr] for [buf_size] bytes with 0xffffffff
    When formatter is used, 0xffffffff is impossible to be emitted. Thus it serves as a marker for trace end.

    Known issue:
        When using the Ram Read Data register on ETR to drain the buffer, the buffer, in theory,
        does not need initialization. However, on some boards, it's observed that if the buffer is
        not written with 0xffffffff, the trace data will be corrupted.
*/
void clear_buffer(uint64_t buf_addr, uint32_t buf_size);

#endif