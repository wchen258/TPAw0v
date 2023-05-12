#include "cs_etm.h"
#include "zcu_cs.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#define ADDR(b, r) ((char *) (&b->r) - (char *) b)

int avail_addr_cmp_high = 7 ;
int avail_addr_cmp_low = 0 ;
int avail_rs_high = 15 ;
int avail_rs_low = 2 ;
int avail_ext_sel_low = 0;
int avail_ext_sel_high = 3;

void etm_implementation_info(ETM_interface *etm)
{
    printf("ETM implementation info:\n");
    printf("CCI minimum: %d\n", etm->id_3 & 0xfff);
    printf("SYNCPR is RO: %d\n", CHECK(etm->id_3, 25));
    printf("Overflow prevention support: %d\n", CHECK(etm->id_3, 31));
    printf("RETSTACK: %d (1 for supported)\n",  CHECK(etm->id_0, 9));
    printf("TRCCOND:  %d (1 for supported)\n",  CHECK(etm->id_0, 6));
    printf("TRCBB:    %d (1 for supported)\n",  CHECK(etm->id_0, 5));

#ifdef DEBUG
    printf("InstView main offset: %lx\n", ADDR(etm, vi_main_ctrl));
    printf("InstView IcEc offset: %lx\n", ADDR(etm, vi_ie_ctrl));
    printf("InstView S-S- offset: %lx\n", ADDR(etm, vi_ss_ctrl));
    printf("Addr comp val [0]off: %lx\n", ADDR(etm, addr_cmp_val[0]));
    printf("Addr comp type[0]off: %lx\n", ADDR(etm, addr_cmp_access_type[0]));
#endif
}

void etm_info(ETM_interface *etm)
{
    printf("Program  Ctrl: 0x%x\n", etm->prog_ctrl);
    printf("Trace  status: 0x%x\n", etm->trace_status);
    printf("OSLock status: 0x%x\n", etm->os_lock_status);
}

void etm_unlock(ETM_interface *etm)
{
    etm->software_lock_access = 0xc5acce55;
    etm->os_lock_access = 0x0;
}

void etm_disable(ETM_interface *p_etm)
{
    p_etm->prog_ctrl = 0x0;
    while( !(p_etm->trace_status & 0x1) );
}

void etm_enable(ETM_interface *p_etm)
{
    p_etm->prog_ctrl = 0x1;
    while( (p_etm->trace_status) & 0x1 );
}

/*
Bare-minimum of trace config, and clear garbage values
*/

void etm_reset(ETM_interface *etm)
{
    etm->trace_config = 0;
    etm->event_ctrl_0 = 0;
    etm->event_ctrl_1 = 0;
    etm->stall_ctrl   = 0;
    etm->sync_period  = 0b10100;
    etm->trace_id     = 0x1;
    etm->global_ts_ctrl = 0;
    etm->vi_main_ctrl = 0x201;
    etm->vi_ie_ctrl = 0;
    etm->vi_ss_ctrl = 0;
    etm->ext_input_sel = 0;

    int i;
    for(i=2; i<32; i++)
        etm->resource_sel_ctrl[i] = 0;  
    
    for(i=0; i<16; i++) {
        etm->addr_cmp_val[i] = 0;
        etm->addr_cmp_access_type[i] = 0;
    }

    for(i=0; i<8; i++) {
        etm->contextid_cmp_val[i] = 0;
        etm->virtual_contextid_cmp_val[i] = 0;
    }

}

int _request_addr_cmp()
{
    if (avail_addr_cmp_high >= avail_addr_cmp_low)
        return avail_addr_cmp_high -- ;
    else {
        fprintf(stderr, "Error: More than 8 Address Comparator requested!\n");
        exit(1);
    }
}

int _request_addr_cmp_pair()
{
    if ((avail_addr_cmp_low + 1) <= avail_addr_cmp_high) {
        int base_pair_num = avail_addr_cmp_low ;
        avail_addr_cmp_low += 2 ;
        return base_pair_num ;
    } else {
        fprintf(stderr, "Error: More than 8 Addreses Comparator requested while requesting a address pair!\n");
        exit(1);
    }
}

int _request_rs()
{
    if (avail_rs_high >= avail_rs_low)
        return avail_rs_high -- ;
    else {
        fprintf(stderr, "Error: More than 14 Resource Selector requeste while requesting a single rs.\n");
        exit(1);
    }
}

int _request_rs_pair()
{
    if ((avail_rs_low + 1) <= avail_rs_high) {
        int base_pair_num = avail_rs_low ;
        avail_rs_low += 2 ;
        return base_pair_num ;
    } else {
        fprintf(stderr, "Error: More than 14 Resource Selector requested while requesting a rs pair!\n");
        exit(1);
    }
}

int _request_ext_sel()
{
    if (avail_ext_sel_high >= avail_ext_sel_low)
        return avail_ext_sel_high -- ;
    else {
        fprintf(stderr, "Error: More than 4 External Input Seletor requeste while requesting a single Ext Sel.\n");
        exit(1);
    }
}

/*
    int cci: [4, 2^12 = 4096]
*/
void etm_set_cci(ETM_interface *etm, int cci)
{
    int ccimin = etm->id_3 & 0xfff;
    if (cci < ccimin) {
        printf("WARNING: requested CCI (%d) is less than CCIMIN (%d), Cycle Count not enabled!\n", cci, ccimin);
        etm->trace_config &= ~(0x1 << 4);
        return ;
    }
    etm->trace_config |= 0x1 << 4 ;
    etm->cycle_count_ctrl = cci ; 
}

/*
    valid p val:
    0 : disable
    0b01000  2^8 bytes per packet
    0b01001  2^9 bytes per packet
    0b01010  2^10 ..
        ...
    0b10100  2^20 ..
*/
void etm_set_sync(ETM_interface *etm, int p)
{
    etm->sync_period = p;
} 


/* level : 0b0 .. 0b1111 , 0b0 means no invasion */
void etm_set_stall(ETM_interface *etm, int level)
{
    if (level) {
        etm->stall_ctrl |= 0x1 << 8;
        etm->stall_ctrl |= 0x1 << 13;
        etm->stall_ctrl |= (level & 0xf);
    }
    else {
        etm->stall_ctrl &= ~(0x1 << 8);
        etm->stall_ctrl &= ~(0x1 << 13);
    }
}

void etm_set_return_stack(ETM_interface *etm)
{
    return ;
}

void etm_set_branch_broadcast(ETM_interface *etm, int inv, uint8_t mask)
{
    SET(etm->trace_config, 3);
    if (inv)
        SET(etm->branch_broadcast_ctrl, 8);
    else
        CLEAR(etm->branch_broadcast_ctrl, 8);
    etm->branch_broadcast_ctrl |= mask;
}

void etm_set_addr_cmp(ETM_interface *etm, int num, uint64_t addr, int cmp_contextid)
{
    etm->addr_cmp_val[num] = addr ;
    if (cmp_contextid) 
        SET(etm->addr_cmp_access_type[num], 2);
    else 
        CLEAR(etm->addr_cmp_access_type[num], 2);
    CLEAR(etm->addr_cmp_access_type[num], 3);
}

void etm_set_range(ETM_interface *etm, int pair_num, uint64_t start_addr, uint64_t end_addr, int cmp_contextid)
{
    int addr_cmp_index = 2 * (pair_num - 1);
    etm_set_addr_cmp(etm, addr_cmp_index, start_addr, cmp_contextid);
    etm_set_addr_cmp(etm, addr_cmp_index + 1, end_addr, cmp_contextid);
    SET(etm->vi_ie_ctrl, pair_num - 1);
}

void etm_register_range(ETM_interface *etm, uint64_t start_addr, uint64_t end_addr, int cmp_contextid)
{
    // why not use the function above.... but why...
    int pair_num = _request_addr_cmp_pair();
    etm_set_range(etm, (pair_num / 2) + 1, start_addr, end_addr, cmp_contextid);
    // int addr_cmp_index_base = _request_addr_cmp_pair();
    // etm_set_addr_cmp(etm, addr_cmp_index_base, start_addr, cmp_contextid);
    // etm_set_addr_cmp(etm, addr_cmp_index_base + 1, end_addr, cmp_contextid);
    // SET(etm->vi_ie_ctrl, addr_cmp_index_base / 2);
}

void etm_register_start_stop_addr(ETM_interface *etm, uint64_t start_addr, uint64_t end_addr)
{
    int cmp_0 = _request_addr_cmp();
    int cmp_1 = _request_addr_cmp();
    etm_set_addr_cmp(etm, cmp_0, start_addr, 1);
    etm_set_addr_cmp(etm, cmp_1, end_addr, 1);
    etm->vi_main_ctrl = 0x1;
    SET(etm->vi_ss_ctrl, cmp_0);
    SET(etm->vi_ss_ctrl, cmp_1 + 16);
}

void etm_set_contextid_cmp(ETM_interface *etm, uint64_t cid)
{
    etm->contextid_cmp_val[0] = cid;
    etm->contextid_cmp_ctrl_0 = 0;
}

/*
    event_bus_num is defined in header with _T in the end
    selector : 0..3
*/
void etm_set_ext_input(ETM_interface *etm, int event_bus_num, int selector)
{
    if (selector > 3 || selector < 0) {
        printf("WARNING: invalid position setting External Input, choose from 0..3\n");
        return ;
    }
    etm->ext_input_sel |= event_bus_num << (8*selector);
}

/*
    rs_num   : Resource Selector number
    rs_group : see header. For PMU event, use External_input
    r1       : According to the group, r1 represents the corresponding sub-resource number
    r2       : only Couter&Sequencer share the same group. Thus if the chosen group is not this, r2 would be ignored
               otherwise, r2 represents the Sequencer number
    inv      : Whether inverse the results
    pair_inv : Whether inverse the combined result from pair resources
*/
void etm_set_rs(ETM_interface *etm, int rs_num, enum rs_group group, int r1, int r2, int inv, int pair_inv)
{
    if (rs_num < 2) {
        printf("WARNING: Resource Selector 0,1 are special RS. Should not be used. RS not set.\n");
        return ;
    }
    if (group == Counter_Seq) {
        SET(etm->resource_sel_ctrl[rs_num], r1);
        SET(etm->resource_sel_ctrl[rs_num], r2 + 4);
    } else 
        SET(etm->resource_sel_ctrl[rs_num], r1);
    etm->resource_sel_ctrl[rs_num] |= group << 16 ;
    if (inv)
        SET(etm->resource_sel_ctrl[rs_num], 20);
    if (pair_inv)
        SET(etm->resource_sel_ctrl[rs_num], 21);
}


void etm_set_event_sel_0(ETM_interface *etm, int rs_num, int pair)
{
    if (rs_num <2)
        printf("WARNING: Resource Selector 0,1 is used for event trace. This is not common, unless intended.\n");
    etm->event_ctrl_0 |= rs_num ;
    if (pair)
        SET(etm->event_ctrl_0, 7);
    else
        CLEAR(etm->event_ctrl_0, 7);
}

void etm_set_event_sel_1(ETM_interface *etm, int rs_num, int pair)
{
    if (rs_num <2)
        printf("WARNING: Resource Selector 0,1 is used for event trace. This is not common, unless intended.\n");
    etm->event_ctrl_0 |= rs_num << 8 ;
    if (pair)
        SET(etm->event_ctrl_0, 15);
    else
        CLEAR(etm->event_ctrl_0, 15);
}

void etm_set_event_sel_2(ETM_interface *etm, int rs_num, int pair)
{
    if (rs_num <2)
        printf("WARNING: Resource Selector 0,1 is used for event trace. This is not common, unless intended.\n");
    etm->event_ctrl_0 |= rs_num << 16 ;
    if (pair)
        SET(etm->event_ctrl_0, 23);
    else
        CLEAR(etm->event_ctrl_0, 23);
}

void etm_set_event_sel_3(ETM_interface *etm, int rs_num, int pair)
{
    if (rs_num <2)
        printf("WARNING: Resource Selector 0,1 is used for event trace. This is not common, unless intended.\n");
    etm->event_ctrl_0 |= rs_num << 24;
    if (pair)
        SET(etm->event_ctrl_0, 31);
    else
        CLEAR(etm->event_ctrl_0, 31);
}

void etm_set_event_sel(ETM_interface *etm, int sel_num, int rs_num, int pair)
{
    switch(sel_num) {
        case 0:
            etm_set_event_sel_0(etm, rs_num, pair);
            break;
        case 1:
            etm_set_event_sel_1(etm, rs_num, pair);
            break;
        case 2:
            etm_set_event_sel_2(etm, rs_num, pair);
            break;
        case 3:
            etm_set_event_sel_3(etm, rs_num, pair);
            break;

        default:
            fprintf(stderr, "ERROR: Invalied Event Selector nuber.\n");
            exit(1);
    }
}

/*
    atb: whether enable atb trigger
*/
void etm_set_event_trc(ETM_interface *etm, int mask, int atb)
{
    etm->event_ctrl_1 |= mask;
    if (atb)
        SET(etm->event_ctrl_1, 11);
    else
        CLEAR(etm->event_ctrl_1, 11);
}

void etm_register_pmu_event(ETM_interface *etm, int event_bus)
{
    int rs_num = _request_rs();
    int ext_num = _request_ext_sel();

    etm_set_ext_input(etm, event_bus, ext_num);
    etm_set_rs(etm, rs_num, External_input, ext_num, 0, 0, 0);
    etm_set_event_sel(etm, ext_num, rs_num, 0);

    // int trc_mask = etm->event_ctrl_1 & 0xf ;
    // trc_mask |= 0x1 << ext_num ;
    // etm_set_event_trc(etm, trc_mask, 0);
    etm_set_event_trc(etm, 0x1 << ext_num, 0);

    printf("External Input: \n    Event Bus Number %d -> Event Packet Pos: %d\n    RS: %d  Ext Sel: %d\n", event_bus, ext_num, rs_num, ext_num);
}

void etm_register_single_addr_match_event(ETM_interface *etm, uint64_t addr) 
{
    int addr_cmp_num = _request_addr_cmp();
    int rs_num = _request_rs();
    int ext_num = _request_ext_sel();

    etm_set_addr_cmp(etm, addr_cmp_num, addr, 1);
    etm_set_rs(etm, rs_num, Single_addr, addr_cmp_num, 0,0,0);
    etm_set_event_sel(etm, ext_num, rs_num, 0);
    etm_set_event_trc(etm, 0x1 << ext_num, 0);

    printf("S.addr  Match: \n     Address: 0x%lx -> Event Packet Pos: %d\n    RS: %d  Ext Sel: %d\n", addr, ext_num, rs_num, ext_num);
}











