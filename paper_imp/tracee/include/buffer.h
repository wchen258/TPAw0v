#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>

uint32_t *get_buf_ptr(uint64_t buf_addr, uint32_t buf_size);
void clear_buffer(uint64_t buf_addr, uint32_t buf_size);
void dump_buffer(uint64_t buf_addr, uint32_t buf_size);


#endif