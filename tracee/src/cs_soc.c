#include "zcu_cs.h"
#include "cs_soc.h"
#include <stdio.h>
#include <stdint.h>

void funnel_config_port(Funnel_interface *funnel, uint8_t mask, int hold_time)
{
    funnel->ctrl = 0;
    funnel->ctrl |= (mask & 0xff) ;
    if (hold_time > 0b1110) {
        printf("WARNING: invalid hold time, choose from 0b0..0b1110. Auto set to 0b0");
    } else {
        funnel->ctrl |= hold_time << 8;
    } 
}

void tmc_set_mode(TMC_interface* tmc, enum tmc_mode mode) {
    tmc->mode = 0x0;
    tmc->mode |= mode;
}

void tmc_set_size(TMC_interface *tmc, uint32_t ram_size)
{
    if(ram_size > 256*1024*1024) {
        printf("WARNING: TMC RAM set size is greater than 256MB\n");
    }
    tmc->ram_size = ram_size/4 ; 
}

void tmc_set_data_buf(TMC_interface *tmc, uint64_t addr)
{
    tmc->data_buf_addr_low = (uint32_t) addr ;
    tmc->data_buf_addr_high = (uint32_t) (addr >> 32);
}

void tmc_set_axi(TMC_interface *tmc, int burst_len)
{
    tmc->axi_ctrl = 0;
    tmc->axi_ctrl |= burst_len & 0xf << 8 ;
    CLEAR(tmc->axi_ctrl, 7);
}

void tmc_set_read_pt(TMC_interface *tmc, uint64_t addr)
{
    tmc->ram_read_pt = (uint32_t) addr;
    tmc->ram_read_pt_high = (uint32_t) (addr >> 32);
}

void tmc_set_write_pt(TMC_interface *tmc, uint64_t addr)
{
    tmc->ram_write_pt = (uint32_t) addr;
    tmc->ram_write_pt_high = (uint32_t) (addr >> 32);
}


