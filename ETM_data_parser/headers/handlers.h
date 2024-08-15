#ifndef HANDLERS_H_  
#define HANDLERS_H_

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

#define CTL_STATE_OUTSCOPE 0
#define CTL_STATE_INSCOPE 1
#define CTL_STATE_POP_COMP 2
#define CTL_STATE_PUSH 3
#define CTL_STATE_INIT 4

#define ADDRESS_STACK_SIZE 1024

typedef struct basicblock {
    uint32_t start_addr: 32;
    uint16_t r: 1;
    uint16_t l: 1;
    uint16_t s: 1;
    uint16_t c: 1;
    uint16_t reserved: 12;
    uint16_t offset: 16;
} __attribute__((packed)) basicblock_t;

void report(const char* format, ... );

void set_ctl_buff(void*, uint16_t);
void report_addres(uint64_t, uint8_t);
void report_atom(uint8_t);

#endif // HANDLERS_H_
