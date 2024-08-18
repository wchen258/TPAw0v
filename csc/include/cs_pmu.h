#ifndef CS_PMU
#define CS_PMU
#include <stdint.h>

#define PMCCNTR_EL0 0xf8
#define PMCCFILTR_EL0 0x47c

#define PAD( start, end ) JOIN( char pad , __COUNTER__ [end - start] )
#define JOIN( symbol1, symbol2 ) _DO_JOIN( symbol1, symbol2 )
#define _DO_JOIN( symbol1, symbol2 ) symbol1##symbol2

typedef struct __attribute__((__packed__)) PMU_interface {
    uint32_t evt_ct_0;
    PAD(0x4, 0x8);
    uint32_t evt_ct_1;
    PAD(0xc,0x10);
    uint32_t evt_ct_2;
    PAD(0x14,0x18);
    uint32_t evt_ct_3;
    PAD(0x1c,0x20);
    uint32_t evt_ct_4;
    PAD(0x24,0x28);
    uint32_t evt_ct_5;
    PAD(0x2c,0xf8);
    uint64_t cc;
    PAD(0x100,0x400);
    uint32_t evt_t_0;
    uint32_t evt_t_1;
    uint32_t evt_t_2;
    uint32_t evt_t_3;
    uint32_t evt_t_4;
    uint32_t evt_t_5;
    PAD(0x418,0x47c);
    uint32_t cc_filter;
    PAD(0x480,0xc00);
    uint32_t ct_en_set;
    PAD(0xc04,0xc20);
    uint32_t ct_en_clear;
    PAD(0xc24,0xc40);
    uint32_t int_en_set;
    PAD(0xc44,0xc60);
    uint32_t int_en_clear;
    PAD(0xc64,0xc80);
    uint32_t overflow_flag_status;
    PAD(0xc84,0xca0);
    uint32_t software_inc;
    PAD(0xca4,0xcc0);
    uint32_t overflow_flag_status_set;
    PAD(0xcc4,0xe00);
    uint32_t config;
    uint32_t ctrl;
    PAD(0xe08,0xe20);
    uint32_t comm_evt_id_0;
    uint32_t comm_evt_id_1;
    PAD(0xe28,0xfa8);
    int32_t dev_aff_0;
    int32_t dev_aff_1;
    int32_t lock_access;
    int32_t lock_status;
    int32_t auth_status;
    int32_t dev_arch;
    PAD(0xfc0,0xfcc);
    int32_t dev_type;
    int32_t peripheral_id_4;
    int32_t peripheral_id_5;
    int32_t peripheral_id_6;
    int32_t peripheral_id_7;
    int32_t peripheral_id_0;
    int32_t peripheral_id_1;
    int32_t peripheral_id_2;
    int32_t peripheral_id_3;
    int32_t comp_id_0;
    int32_t comp_id_1;
    int32_t comp_id_2;
    int32_t comp_id_3;


} PMU_interface ;


#endif