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
    tmc->mode = mode;
}

void tmc_set_size(TMC_interface *tmc, uint32_t ram_size)
{
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

// Below are functions for debug purpose

void cti_report(CTI_interface *cti) {
    printf("CTI report Start\n");
    printf("ctrl               %x\n", cti->ctrl);
    printf("Trigger In Status  %x\n", cti->trig_in_status);
    printf("trigger out status %x\n", cti->trig_out_status);
    printf("Channel In Status  %x\n", cti->channel_in_status);
    printf("Channel Out Status %x\n", cti->channel_out_status);
}


void replicator_report(Replicator_interface* repl) {
    printf("Replicator report Start\n");
    printf("id_filter_atb_master_p_0 %x\n", repl->id_filter_atb_master_p_0);
    printf("id_filter_atb_master_p_1 %x\n", repl->id_filter_atb_master_p_1);
    printf("integration_mode_atb_ctrl_0 %x\n", repl->integration_mode_atb_ctrl_0);
    printf("integration_mode_atb_ctrl_1 %x\n", repl->integration_mode_atb_ctrl_1);
    printf("integration_mode_ctrl %x\n", repl->integration_mode_ctrl);
    printf("claim_tag_set %x\n", repl->claim_tag_set);
    printf("claim_tag_clear %x\n", repl->claim_tag_clear);
    printf("lock_access %x\n", repl->lock_access);
    printf("lock_status %x\n", repl->lock_status);
    printf("auth_status %x\n", repl->auth_status);
    printf("dev_config %x\n", repl->dev_config);
    printf("dev_type_identifier %x\n", repl->dev_type_identifier);
    printf("peri_id_4 %x\n", repl->peri_id_4);
    printf("peri_id_0 %x\n", repl->peri_id_0);
    printf("peri_id_1 %x\n", repl->peri_id_1);
    printf("peri_id_2 %x\n", repl->peri_id_2);
    printf("peri_id_3 %x\n", repl->peri_id_3);
    printf("comp_id_0 %x\n", repl->comp_id_0);
    printf("comp_id_1 %x\n", repl->comp_id_1);
    printf("comp_id_2 %x\n", repl->comp_id_2);
    printf("comp_id_3 %x\n", repl->comp_id_3);
}

void explain_tmc_STS(uint32_t sts_reg) {
    printf("==== TMC STS register ====\n");
    printf("Value %x\n", sts_reg);
    printf("Full %d\n", (sts_reg & 0x1));
    printf("Triggered %d\n", (sts_reg & 0x2) >> 1);
    printf("TMCReady %d\n", (sts_reg & 0x4) >> 2);
    printf("FtEmpty %d\n", (sts_reg & 0x8) >> 3);  
    printf("Empty %d\n", (sts_reg & 0x10) >> 4);
    printf("MemErr %d\n", (sts_reg & 0x20) >> 5);
    printf("==== END ====\n");
}

void explain_tmc_FFSR(uint32_t ffsr_reg) {
    printf("==== TMC FFSR register ====\n");
    printf("Value %x\n", ffsr_reg);
    printf("FlInProg[0] %d\n", (ffsr_reg & 0x1));
    printf("FtStopped[1] (same as FtEmpty in STS) %d\n", (ffsr_reg & 0x2) >> 1);
    printf("==== END ====\n");
}

void tmc_report(TMC_interface* tmc, int tmc_index) {
    printf("**** TMC %d report Start ****\n", tmc_index);
    printf("ram_size %x\n", tmc->ram_size);
    // printf("status %x\n", tmc->status);
    explain_tmc_STS(tmc->status);
    printf("ram_read_pt %x\n", tmc->ram_read_pt);
    printf("ram_write_pt %x\n", tmc->ram_write_pt);
    printf("trig_counter %x\n", tmc->trig_counter);
    printf("ctrl %x\n", tmc->ctrl);
    // printf("ram_write_data %x\n", tmc->ram_write_data);
    // printf("mode %x\n", tmc->mode);
    // printf("latched_buf_fill_level %x\n", tmc->latched_buf_fill_level);
    // printf("cur_buf_fill_level %x\n", tmc->cur_buf_fill_level);
    // printf("buf_level_water_mark %x\n", tmc->buf_level_water_mark);
    printf("ram_read_pt_high %x\n", tmc->ram_read_pt_high);
    printf("ram_write_pt_high %x\n", tmc->ram_write_pt_high);
    printf("axi_ctrl %x\n", tmc->axi_ctrl);
    // printf("data_buf_addr_low %x\n", tmc->data_buf_addr_low);
    // printf("data_buf_addr_high %x\n", tmc->data_buf_addr_high);
    // printf("formatter_flush_status %x\n", tmc->formatter_flush_status);
    explain_tmc_FFSR(tmc->formatter_flush_status);
    printf("formatter_flush_ctrl %x\n", tmc->formatter_flush_ctrl);
    // printf("periodic_sync_counter %x\n", tmc->periodic_sync_counter);
    printf("**** TMC %d DONE ****\n\n", tmc_index);
}

/*
    Calling this function in full will cause the execution to hange.
    The board is not killed though.
    It's very likely the TPIU is not powered. 
    According to manul, we need to PHYSICALLY connect two jumps on board. 

    If this is true, then TPIU is sliently exerting a upstream pressure to TMC2
    that explains why TMC2 is stunned.
*/
void tpiu_report(TPIU_interface* tpiu) {
    printf("TPIU report Start\n");
    printf("support_port_size %x\n", tpiu->support_port_size);
    // printf("cur_port_size %x\n", tpiu->cur_port_size);
    // printf("support_trig_mode %x\n", tpiu->support_trig_mode);
    // printf("trig_counter_val %x\n", tpiu->trig_counter_val);
    // printf("trig_multiplier %x\n", tpiu->trig_multiplier);
    // printf("support_test_pattern_mode %x\n", tpiu->support_test_pattern_mode);
    // printf("cur_test_pattern_mode %x\n", tpiu->cur_test_pattern_mode);
    // printf("tpiu_test_pattern_repeat_counter %x\n", tpiu->tpiu_test_pattern_repeat_counter);
    // printf("formatter_flush_status %x\n", tpiu->formatter_flush_status);
    // printf("formatter_flush_ctrl %x\n", tpiu->formatter_flush_ctrl);
    // printf("formatter_sync_counter %x\n", tpiu->formatter_sync_counter);
    // printf("tpiu_exctl_in_port %x\n", tpiu->tpiu_exctl_in_port);
    // printf("tpiu_exctl_out_port %x\n", tpiu->tpiu_exctl_out_port);
    // printf("integration_test_trig_in_flush_in_ack %x\n", tpiu->integration_test_trig_in_flush_in_ack);
    // printf("integration_test_trig_in_flush_in %x\n", tpiu->integration_test_trig_in_flush_in);
    // printf("integration_test_atb_data_0 %x\n", tpiu->integration_test_atb_data_0);
    // printf("integration_test_ctrl_2 %x\n", tpiu->integration_test_ctrl_2);
    // printf("integration_test_ctrl_1 %x\n", tpiu->integration_test_ctrl_1);
    // printf("integration_test_ctrl_0 %x\n", tpiu->integration_test_ctrl_0);
    // printf("integration_mode_ctrl %x\n", tpiu->integration_mode_ctrl);
    // printf("claim_tag_set %x\n", tpiu->claim_tag_set);
    // printf("claim_tag_clear %x\n", tpiu->claim_tag_clear);
    printf("lock_access %x\n", tpiu->lock_access);
    printf("lock_status %x\n", tpiu->lock_status);
}
