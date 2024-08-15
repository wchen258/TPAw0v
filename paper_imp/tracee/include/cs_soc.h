#ifndef CS_SOC_H
#define CS_SOC_H

#include <stdint.h>

#ifdef DEBUG
#include <stdio.h>
#endif

#define PAD( start, end ) JOIN( char pad , __COUNTER__ [end - start] )
#define JOIN( symbol1, symbol2 ) _DO_JOIN( symbol1, symbol2 )
#define _DO_JOIN( symbol1, symbol2 ) symbol1##symbol2

enum tmc_mode {Circular, Soft, Hard} ;

typedef struct __attribute__((__packed__)) funnel_interface {
    uint32_t ctrl;
    uint32_t priority_ctrl;
    PAD(0x8, 0xEEC);
    uint32_t integration_test_atb_data_0;
    uint32_t integration_test_atb_ctrl_2;
    uint32_t integration_test_atb_ctrl_1;
    uint32_t integration_test_atb_ctrl_0;
    PAD(0xefc, 0xf00);
    uint32_t integration_mode_ctrl;
    PAD(0xF04, 0xFA0);
    uint32_t claim_tag_set;
    uint32_t claim_tag_clear;
    PAD(0xFA8, 0xFB0);
    uint32_t lock_access;
    uint32_t lock_status;
    uint32_t auth_status;
    PAD(0xFBC, 0xFC8);
    uint32_t dev_config;
    uint32_t dev_type_identifier;
    uint32_t peri_id_4;
    PAD(0xFD4, 0xFE0);
    uint32_t peri_id_0;
    uint32_t peri_id_1;
    uint32_t peri_id_2;
    uint32_t peri_id_3;
    uint32_t comp_id_0;
    uint32_t comp_id_1;
    uint32_t comp_id_2;
    uint32_t comp_id_3;
} Funnel_interface ;

typedef struct __attribute__((__packed__)) replicator_interface {
    uint32_t id_filter_atb_master_p_0;
    uint32_t id_filter_atb_master_p_1;
    PAD(0x8, 0xEF8);
    uint32_t integration_mode_atb_ctrl_0;
    uint32_t integration_mode_atb_ctrl_1;
    uint32_t integration_mode_ctrl ;
    PAD(0xF04, 0xFA0);
    uint32_t claim_tag_set ;
    uint32_t claim_tag_clear ;
    PAD(0xFA8, 0xFB0);
    uint32_t lock_access ;
    uint32_t lock_status ;
    uint32_t auth_status ;
    PAD(0xFBC, 0xFC8);
    uint32_t dev_config ;
    uint32_t dev_type_identifier;
    uint32_t peri_id_4;
    PAD(0xFD4, 0xFE0);
    uint32_t peri_id_0 ;
    uint32_t peri_id_1 ;
    uint32_t peri_id_2 ;
    uint32_t peri_id_3 ;
    uint32_t comp_id_0 ;
    uint32_t comp_id_1 ;
    uint32_t comp_id_2 ;
    uint32_t comp_id_3 ;
} Replicator_interface ;

typedef struct __attribute__((__packed__)) tpiu_interface {
    uint32_t support_port_size;
    uint32_t cur_port_size ;
    PAD(0x8, 0x100);
    uint32_t support_trig_mode;
    uint32_t trig_counter_val ;
    uint32_t trig_multiplier ;
    PAD(0x10c, 0x200);
    uint32_t support_test_pattern_mode;
    uint32_t cur_test_pattern_mode;
    uint32_t tpiu_test_pattern_repeat_counter;
    PAD(0x20c, 0x300);
    uint32_t formatter_flush_status;
    uint32_t formatter_flush_ctrl ;
    uint32_t formatter_sync_counter;
    PAD(0x30c, 0x400);
    uint32_t tpiu_exctl_in_port ;
    uint32_t tpiu_exctl_out_port ;  
    PAD(0x40c, 0xee4);
    uint32_t integration_test_trig_in_flush_in_ack ;
    uint32_t integration_test_trig_in_flush_in ;
    uint32_t integration_test_atb_data_0 ;
    uint32_t integration_test_ctrl_2 ;
    uint32_t integration_test_ctrl_1 ;
    uint32_t integration_test_ctrl_0 ;
    uint32_t integration_mode_ctrl ;
    PAD(0xf04, 0xfa0);
    uint32_t claim_tag_set ;
    uint32_t claim_tag_clear ;
    PAD(0xfa8, 0xfb0);
    uint32_t lock_access ;
    uint32_t lock_status ;
    uint32_t auth_status ;
    PAD(0xfbc, 0xfc8);
    uint32_t dev_config;
    uint32_t dev_type_identifier;
    uint32_t peri_id_4;
    PAD(0xfd4, 0xfe0);
    uint32_t peri_id_0;
    uint32_t peri_id_1;
    uint32_t peri_id_2;
    uint32_t peri_id_3;
    uint32_t comp_id_0;
    uint32_t comp_id_1;
    uint32_t comp_id_2;
    uint32_t comp_id_3;
} TPIU_interface ;

typedef struct __attribute__((__packed__)) cti_interface {
    uint32_t ctrl;
    PAD(0x4, 0x10);
    uint32_t interrupt_ack;
    uint32_t app_trig_set;
    uint32_t app_trig_clear;
    uint32_t app_pulse;
    uint32_t trig_to_channel_en [8];
    PAD(0x40, 0xa0);
    uint32_t channel_to_trig_en [8];
    PAD(0xc0, 0x130);
    uint32_t trig_in_status;
    uint32_t trig_out_status;
    uint32_t channel_in_status;
    uint32_t channel_out_status;
    uint32_t en_cti_channel_gate;
    uint32_t ext_multiplexer_ctrl ;
    PAD(0x148, 0xedc);
    uint32_t integration_test_channel_input_ack;
    uint32_t integration_test_trig_input_ack;
    uint32_t integration_test_channel_output;
    uint32_t integration_test_trig_output;
    uint32_t integration_test_channel_output_ack;
    uint32_t integration_test_trig_output_ack;
    uint32_t integration_test_channel_input ;
    uint32_t integration_test_trig_input;
    uint32_t integration_mode_ctrl;
    PAD(0xf04, 0xfa0);
    uint32_t claim_tag_set ;
    uint32_t claim_tag_clear ;
    PAD(0xfa8, 0xfb0);
    uint32_t lock_access ;
    uint32_t lock_status ;
    uint32_t auth_status ;
    PAD(0xfbc, 0xfc8);
    uint32_t dev_config;
    uint32_t dev_type_identifier;
    uint32_t peri_id_4;
    PAD(0xfd4, 0xfe0);
    uint32_t peri_id_0;
    uint32_t peri_id_1;
    uint32_t peri_id_2;
    uint32_t peri_id_3;
    uint32_t comp_id_0;
    uint32_t comp_id_1;
    uint32_t comp_id_2;
    uint32_t comp_id_3;
} CTI_interface ;

typedef struct __attribute__((__packed__)) tmc_interface {
    PAD(0x0, 0x4);
    uint32_t ram_size ;
    PAD(0x8, 0xc);
    uint32_t status ;
    uint32_t ram_read_data ;
    uint32_t ram_read_pt ;
    uint32_t ram_write_pt ; 
    uint32_t trig_counter ;
    uint32_t ctrl ;
    uint32_t ram_write_data;
    uint32_t mode;
    uint32_t latched_buf_fill_level ;
    uint32_t cur_buf_fill_level ;
    uint32_t buf_level_water_mark ;
    uint32_t ram_read_pt_high;
    uint32_t ram_write_pt_high;
    PAD(0x40, 0x110);
    uint32_t axi_ctrl;
    PAD(0x114, 0x118);
    uint32_t data_buf_addr_low;
    uint32_t data_buf_addr_high;
    PAD(0x120, 0x300);
    uint32_t formatter_flush_status;
    uint32_t formatter_flush_ctrl;
    uint32_t periodic_sync_counter ;
    PAD(0x30c, 0xed0);
    uint32_t integration_test_atb_master_data_0;
    uint32_t integration_test_atb_master_interface_ctrl_2;
    uint32_t integration_test_atb_master_ctrl_1 ;
    uint32_t integration_test_atb_master_interface_ctrl_0 ;
    uint32_t integration_test_misc_output_0;
    PAD(0xee4, 0xee8);
    uint32_t integration_test_trig_in_flush_in ;
    uint32_t integration_test_atb_data_0;
    uint32_t integration_test_atb_ctrl_2;
    uint32_t integration_test_atb_ctrl_1;
    uint32_t integration_test_atb_ctrl_0;
    PAD(0xefc, 0xf00);
    uint32_t integration_mode_ctrl ;
    PAD(0xf04, 0xfa0);
    uint32_t claim_tag_set ;
    uint32_t claim_tag_clear ;
    PAD(0xfa8, 0xfb0);
    uint32_t lock_access ;
    uint32_t lock_status ;
    uint32_t auth_status ;
    PAD(0xfbc, 0xfc8);
    uint32_t dev_config;
    uint32_t dev_type_identifier;
    uint32_t peri_id_4;
    uint32_t peri_id_5;
    uint32_t peri_id_6;
    uint32_t peri_id_7;
    uint32_t peri_id_0;
    uint32_t peri_id_1;
    uint32_t peri_id_2;
    uint32_t peri_id_3;
    uint32_t comp_id_0;
    uint32_t comp_id_1;
    uint32_t comp_id_2;
    uint32_t comp_id_3;
} TMC_interface ;

static inline void funnel_unlock(Funnel_interface *funnel)
{
    funnel->lock_access = 0xc5acce55;
}

static inline void replicator_unlock(Replicator_interface *replicator)
{
    replicator->lock_access = 0xc5acce55;
}

static inline void tmc_unlock(TMC_interface *tmc)
{
    tmc->lock_access = 0xc5acce55;
}

static inline void tpiu_unlock(TPIU_interface *tpiu)
{
    tpiu->lock_access = 0xc5acce55;
}

static inline void cti_unlock(CTI_interface *cti)
{
    cti->lock_access = 0xc5acce55;
}

static inline void tmc_enable(TMC_interface *tmc)
{
    tmc->ctrl = 0x1;
}

static inline void tmc_disable(TMC_interface *tmc)
{
    tmc->ctrl = 0x0;
}

static inline void tmc_man_flush(TMC_interface *tmc)
{
    tmc->formatter_flush_ctrl |= 0x1 << 6;
}

void tmc_set_mode(TMC_interface *, enum tmc_mode);
void tmc_set_size(TMC_interface *, uint32_t);
void tmc_set_data_buf(TMC_interface *, uint64_t);
void tmc_set_axi(TMC_interface *, int);
void tmc_set_read_pt(TMC_interface *, uint64_t);
void tmc_set_write_pt(TMC_interface *, uint64_t);
void funnel_config_port(Funnel_interface *funnel, uint8_t mask, int hold_time);





#endif