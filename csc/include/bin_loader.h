#ifndef BIN_LOADER_H
#define BIN_LOADER_H


#include <stdint.h>

#define TMG_0_ADDR 0xFFEB2020
#define TMG_1_ADDR 0xFFEB2420
#define TMG_2_ADDR 0xFFEB2820
#define TMG_3_ADDR 0xFFEB2C20

uint32_t wrmem(char* bin_name, unsigned long addr);

#endif



