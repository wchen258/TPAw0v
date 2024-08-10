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
    // tmc->axi_ctrl = 0;
    // tmc->axi_ctrl |= burst_len & 0xf << 8 ;
    // CLEAR(tmc->axi_ctrl, 7);

    // According to manual, 0b111111 is common case
    tmc->axi_ctrl = 0b111111;
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


void cti_config(CTI_interface *tar_cti, uint32_t gate_mask) {
    cti_unlock(tar_cti);  // unlock CTI to enable write
    tar_cti->ctrl = 0x0u;
    while(tar_cti->ctrl != 0x0u) {;};

    // clear out all the trigin to channel
    tar_cti->trig_to_channel_en[0] = 0x0u;
    tar_cti->trig_to_channel_en[1] = 0x0u;
    tar_cti->trig_to_channel_en[2] = 0x0u;
    tar_cti->trig_to_channel_en[3] = 0x0u;
    tar_cti->trig_to_channel_en[4] = 0x0u;
    tar_cti->trig_to_channel_en[5] = 0x0u;
    tar_cti->trig_to_channel_en[6] = 0x0u;
    tar_cti->trig_to_channel_en[7] = 0x0u;
 
    // clear out all the channel to trigout
    tar_cti->channel_to_trig_en[0] = 0x0u;
    tar_cti->channel_to_trig_en[1] = 0x0u;
    tar_cti->channel_to_trig_en[2] = 0x0u;
    tar_cti->channel_to_trig_en[3] = 0x0u;
    tar_cti->channel_to_trig_en[4] = 0x0u;
    tar_cti->channel_to_trig_en[5] = 0x0u;
    tar_cti->channel_to_trig_en[6] = 0x0u;
    tar_cti->channel_to_trig_en[7] = 0x0u;

    tar_cti->en_cti_channel_gate = gate_mask;

    // acknowledge all the standout interrupts
    tar_cti->interrupt_ack = ~0;


    /*  the following setting enables channel 0 to trigger output 0
        Output 0 is hardwared to debug request */
    // tar_cti->channel_to_trig_en[0] = 0b1;

    // enable CTI
    tar_cti->ctrl = 0x1u;

}


void cti_report(CTI_interface *cti) {
    printf("CTI report Start\n");
    printf("ctrl               %x\n", cti->ctrl);
    printf("Trigger In Status  %x\n", cti->trig_in_status);
    printf("trigger out status %x\n", cti->trig_out_status);
    printf("Channel In Status  %x\n", cti->channel_in_status);
    printf("Channel Out Status %x\n", cti->channel_out_status);
}