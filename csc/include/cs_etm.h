#include <stdint.h>

#ifndef CS_COMPONENT_H
#define CS_COMPONENT_H

#define TRCPRGCTLR		0x004
#define TRCSTATR		0x00c
#define TRCLAR			0xfb0
#define TRCLSR			0xfb4
#define TRCOSLAR		0x300
#define TRCOSLSR		0x304
#define TRCCONFIGR		0x010 
#define TRCEVENTCTL0R 	0x020
#define TRCEVENTCTL1R 	0x024
#define TRCSTALLCTLR 	0x02c
#define TRCSYNCPR 		0x034
#define TRCTRACEIDR		0x040
#define TRCTSCTLR 		0x030
#define TRCVICTLR 		0x080
#define TRCVIIECTLR 	0x084
#define TRCVISSCTLR		0x088
#define TRCCCCTLR       0x038
#define TRCEXTINSELR    0x120  
#define TRCRSCTLR0      0x200
#define TRCRSCTLR1      0x204
#define TRCRSCTLR2      0x208
#define TRCRSCTLR3      0x20c
#define TRCRSCTLR4      0x210
#define TRCRSCTLR5      0x214
#define TRCRSCTLR6      0x218
#define TRCRSCTLR7      0x21c
#define TRCRSCTLR8      0x220
#define TRCRSCTLR9      0x224
#define TRCRSCTLR10     0x228
#define TRCRSCTLR11     0x22c
#define TRCRSCTLR12     0x230
#define TRCRSCTLR13     0x234
#define TRCRSCTLR14     0x238
#define TRCRSCTLR15     0x23c                 

// Address Comparator Value Register
#define TRCACVR0  0x400
#define TRCACVR1  0x408
#define TRCACVR2  0x410
#define TRCACVR3  0x418
#define TRCACVR4  0x420
#define TRCACVR5  0x428
#define TRCACVR6  0x430
#define TRCACVR7  0x438
#define TRCACVR8  0x440
#define TRCACVR9  0x448
#define TRCACVR10   0x450
#define TRCACVR11   0x458
#define TRCACVR12   0x460
#define TRCACVR13   0x468
#define TRCACVR14   0x470
#define TRCACVR15   0x478

// Address Comparator Type Register
#define TRCACATR0  0x480
#define TRCACATR1  0x488
#define TRCACATR2  0x490
#define TRCACATR3  0x498
#define TRCACATR4  0x4a0
#define TRCACATR5  0x4a8
#define TRCACATR6  0x4b0
#define TRCACATR7  0x4b8
#define TRCACATR8  0x4c0
#define TRCACATR9  0x4c8
#define TRCACATR10   0x4d0
#define TRCACATR11   0x4d8
#define TRCACATR12   0x4e0
#define TRCACATR13   0x4e8
#define TRCACATR14   0x4f0
#define TRCACATR15   0x4f8

// Context ID comparator, Cortex-A53 has only one
#define TRCCIDCVRD0 0x600
#define TRCCIDCCTLR 0x680

#define PAD( start, end ) JOIN( char pad , __COUNTER__ [end - start] )
#define JOIN( symbol1, symbol2 ) _DO_JOIN( symbol1, symbol2 )
#define _DO_JOIN( symbol1, symbol2 ) symbol1##symbol2

enum rs_group {External_input, PE_comparator, Counter_Seq, Single_shot, Single_addr, Addr_Range, Contextid, Virtual_context};


typedef struct __attribute__((__packed__)) etm_interface {
    PAD(0x0, 0x4);
    uint32_t prog_ctrl;
    uint32_t pe_sel_ctrl;
    uint32_t trace_status;
    uint32_t trace_config;
    uint32_t res1;
    uint32_t aux_ctrl;
    uint32_t res2;
    uint32_t event_ctrl_0;
    uint32_t event_ctrl_1;
    uint32_t res3;
    uint32_t stall_ctrl;
    uint32_t global_ts_ctrl;
    uint32_t sync_period;
    uint32_t cycle_count_ctrl;
    uint32_t branch_broadcast_ctrl;
    uint32_t trace_id;
    uint32_t q_element_ctrl;
    PAD(0x48, 0x080);
    uint32_t vi_main_ctrl;
    uint32_t vi_ie_ctrl;
    uint32_t vi_ss_ctrl;
    uint32_t vi_ss_pe_cmp_ctrl;
    PAD(0x90, 0xa0);
    uint32_t vd_main_ctrl;
    uint32_t vd_ii_single_addr_cmp_ctrl;
    uint32_t vd_ii_addr_range_cmp_ctrl;
    PAD(0xac, 0x100);
    uint32_t sequencer_state_transition_ctrl [3];
    PAD(0x10c, 0x118);
    uint32_t sequencer_rst_ctrl;
    uint32_t sequencer_state;
    uint32_t ext_input_sel;
    PAD(0x124, 0x140);
    uint32_t counter_reload_val [4];
    uint32_t counter_ctrl [4];
    uint32_t counter_val [4];
    PAD(0x170, 0x180);
    uint32_t id_8;
    uint32_t id_9;
    uint32_t id_10;
    uint32_t id_11;
    uint32_t id_12;
    uint32_t id_13;
    PAD(0x198, 0x1c0);
    uint32_t implementation_defined_r [8] ;     
    uint32_t id_0;
    uint32_t id_1;
    uint32_t id_2;
    uint32_t id_3;
    uint32_t id_4;
    uint32_t id_5;
    uint32_t id_6;
    uint32_t id_7;
    uint32_t resource_sel_ctrl [32];
    uint32_t single_shot_cmp_ctrl [8];
    uint32_t single_shot_cmp_status [8];
    uint32_t single_shot_cmp_input_ctrl [8];
    PAD(0x2e0, 0x300);
    uint32_t os_lock_access;
    uint32_t os_lock_status;
    PAD(0x308, 0x310);
    uint32_t powerdown_ctrl;
    uint32_t powerdown_status;
    PAD(0x318, 0x380);
    uint32_t res_blk7 [32];
    uint64_t addr_cmp_val [16];
    uint64_t addr_cmp_access_type [16];
    
    /*
    Data Trace Block.
    Ignore them since Cortex-A53 does not support Data Trace
    */

    uint64_t data_regs [32];

    uint64_t contextid_cmp_val [8];
    uint64_t virtual_contextid_cmp_val [8];
    uint32_t contextid_cmp_ctrl_0 ;
    uint32_t contextid_cmp_ctrl_1 ;
    uint32_t virtual_contextid_cmp_ctrl_0;
    uint32_t virtual_contextid_cmp_ctrl_1;
    uint32_t res_blk13 [28];
    uint32_t res_blk14_28 [480];
    uint32_t res_imp_def_topology_detection [32];
    uint32_t integration_mode_ctrl ;
    uint32_t res_blk30 [31];
    uint32_t res_blk31_a [8];
    uint32_t claim_tag_set ;
    uint32_t claim_tag_clear ;
    uint32_t dev_affinity_0;
    uint32_t dev_affinity_1;
    uint32_t software_lock_access ;
    uint32_t software_lock_status ;
    uint32_t auth_status ;
    uint32_t dev_arch ;
    uint32_t res_blk31_b [2];
    uint32_t dev_id;
    uint32_t dev_type;
    uint32_t peri_id_4 ;
    uint32_t peri_id_5 ;
    uint32_t peri_id_6 ;
    uint32_t peri_id_7 ;
    uint32_t peri_id_0 ;
    uint32_t peri_id_1 ;
    uint32_t peri_id_2 ;
    uint32_t peri_id_3 ;
    uint32_t comp_id_0 ;
    uint32_t comp_id_1 ;
    uint32_t comp_id_2 ;
    uint32_t comp_id_3 ;

} ETM_interface ;

static inline void etm_set_cid(ETM_interface *etm)
{
    etm->trace_config |= 0x1 << 6 ;
}

ETM_interface* etm_register(int);
void etm_disable(ETM_interface *);
void etm_enable(ETM_interface *);
uint8_t etm_is_idle(ETM_interface *p_etm);
void etm_info(ETM_interface *);
void etm_set_cci(ETM_interface* , int);
void etm_set_sync(ETM_interface*, int);
void etm_implementation_info(ETM_interface*);
void etm_unlock(ETM_interface*);
void etm_reset(ETM_interface *);
void etm_set_stall(ETM_interface*, int);
void etm_set_branch_broadcast(ETM_interface*, int, uint8_t);
void etm_set_contextid_cmp(ETM_interface*, uint64_t);
void etm_set_ext_input(ETM_interface*, int, int);
void etm_set_event_trc(ETM_interface*, int mask, int atb);
void etm_register_pmu_event(ETM_interface *, int event_bus);
void etm_register_range(ETM_interface*, uint64_t start_addr, uint64_t end_addr, int cmp_contextid);
void etm_register_single_addr_match_event(ETM_interface *, uint64_t);
void etm_register_start_stop_addr(ETM_interface *etm, uint64_t start_addr, uint64_t end_addr);
void etm_example_single_counter(ETM_interface* etm, int event_bus, uint16_t counter_val);
void etm_example_large_counter(ETM_interface* etm, int event_bus, uint32_t counter_val);
void etm_print_large_counter(ETM_interface* etm, int cnt_base_index);
void etm_example_single_counter_fire_event(ETM_interface* etm, int event_bus, uint16_t counter_val);
void etm_example_large_counter_fire_event(ETM_interface* etm, int event_bus, uint32_t counter_val);
#endif
