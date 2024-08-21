#ifndef COMMON_H_
#define COMMON_H_

#include "common.h"
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
int buff_read();
void spawn_child(void (*func)());
void poller();
uint32_t wrmem(char* bin_name, unsigned long addr);

#endif