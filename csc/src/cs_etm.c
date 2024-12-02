#include "cs_etm.h"
#include "zcu_cs.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <assert.h>

#define ADDR(b, r) ((char *) (&b->r) - (char *) b)

/*
    Resource and Address Comparator can be used either as single or pair
    When using as pair, the even index (starting from 0) and the next odd index are used as a pair
    Thus when making a request, it's advised to use the _request function below
    When requesting pair, the index is taken from below
    When requesting single, the index is taken from above to resolve the conflict

    resource index == 0 is always False, and == 1 is always True
*/
int avail_addr_cmp_high[4] = {7,7,7,7} ;
int avail_addr_cmp_low[4] = {0,0,0,0} ;
int avail_rs_high[4] = {15,15,15,15} ;
int avail_rs_low[4] = {2,2,2,2} ;
int avail_ext_sel_low[4] = {0,0,0,0};
int avail_ext_sel_high[4] = {3,3,3,3};
int avail_ct_low[4] = {0,0,0,0};
uint8_t avail_ct_high[4] = {1,1,1,1};

extern volatile ETM_interface* etms[4];

static int get_etm_index(volatile ETM_interface* etm)
{
    int i;
    for(i=0; i<4; i++) {
        if (etms[i] == etm)
            return i;
    }
    return -1;
}

static uint8_t _request_ct(volatile ETM_interface* etm)
{
    int id = get_etm_index(etm);
    if (avail_ct_high[id] >= avail_ct_low[id])
        return avail_ct_low[id] ++ ;
    else {
        fprintf(stderr, "Error: More than 2 Counter requested!\n");
        exit(1);
    }
}

static int _request_addr_cmp(volatile ETM_interface* etm)
{
    int id = get_etm_index(etm);
    if (avail_addr_cmp_high[id] >= avail_addr_cmp_low[id])
        return avail_addr_cmp_high[id] -- ;
    else {
        fprintf(stderr, "Error: More than 8 Address Comparator requested!\n");
        exit(1);
    }
}

/*
    Check if there is enough address comparator available
    if so, return the index of the first address comparator
*/
static int _request_addr_cmp_pair(volatile ETM_interface* etm)
{
    int id = get_etm_index(etm);
    if ((avail_addr_cmp_low[id] + 1) <= avail_addr_cmp_high[id]) {
        int base_pair_num = avail_addr_cmp_low[id] ;
        avail_addr_cmp_low[id] += 2 ;
        return base_pair_num ;
    } else {
        fprintf(stderr, "Error: More than 8 Addreses Comparator requested while requesting a address pair!\n");
        exit(1);
    }
}

static int _request_rs(volatile ETM_interface* etm)
{
    int id = get_etm_index(etm);
    if (avail_rs_high[id] >= avail_rs_low[id])
        return avail_rs_high[id] -- ;
    else {
        fprintf(stderr, "Error: More than 14 Resource Selector requeste while requesting a single rs.\n");
        exit(1);
    }
}

static int _request_rs_pair(volatile ETM_interface* etm)
{
    int id = get_etm_index(etm);
    if ((avail_rs_low[id] + 1) <= avail_rs_high[id]) {
        int base_pair_num = avail_rs_low[id] ;
        avail_rs_low[id] += 2 ;
        return base_pair_num ;
    } else {
        fprintf(stderr, "Error: More than 14 Resource Selector requested while requesting a rs pair!\n");
        exit(1);
    }
}

/* return next available External Input Selector index */
static int _request_ext_sel(volatile ETM_interface* etm)
{
    int id = get_etm_index(etm);
    if (avail_ext_sel_high[id] >= avail_ext_sel_low[id])
        return avail_ext_sel_high[id] -- ;
    else {
        fprintf(stderr, "Error: More than 4 External Input Seletor requeste while requesting a single Ext Sel.\n");
        exit(1);
    }
}

void etm_implementation_info(volatile ETM_interface *etm)
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

void etm_info(volatile ETM_interface *etm)
{
    printf("Program  Ctrl: 0x%x\n", etm->prog_ctrl);
    printf("Trace  status: 0x%x\n", etm->trace_status);
    printf("OSLock status: 0x%x\n", etm->os_lock_status);
}

void etm_unlock(volatile ETM_interface *etm)
{
    etm->software_lock_access = 0xc5acce55;
    etm->os_lock_access = 0x0;
}

void etm_disable(volatile ETM_interface *p_etm)
{
    p_etm->prog_ctrl = 0x0;
    while( !(p_etm->trace_status & 0x1) );
}

void etm_enable(volatile ETM_interface *p_etm)
{
    p_etm->prog_ctrl = 0x1;
    while( (p_etm->trace_status) & 0x1 );
}

uint8_t etm_is_idle(volatile ETM_interface *p_etm)
{
    return (p_etm->trace_status & 0x1);
}


void etm_reset(volatile ETM_interface *etm)
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

    for(i=0; i<4; i++) {
        etm->counter_ctrl[i] = 0;
        etm->counter_reload_val[i] = 0;
        etm->counter_val[i] = 0;
    }

}

void etm_set_contextid_cmp(volatile ETM_interface *etm, uint64_t cid)
{
    etm->contextid_cmp_val[0] = cid;
    etm->contextid_cmp_ctrl_0 = 0;
}

/*
    event_bus_num is defined in header with _T in the end
    the bus number need to be added by 4, because the first 4 are used for CTI.
    Arm recommands implment performance counter bus tom [n+3:4]
    selector : 0..3
*/
void etm_set_ext_input(volatile ETM_interface *etm, int event_bus_num, int selector)
{
    if (selector > 3 || selector < 0) {
        printf("WARNING: invalid position setting External Input, choose from 0..3\n");
        return ;
    }
    etm->ext_input_sel |= (event_bus_num + 4) << (8*selector);
}


/*
    int cci: [4, 2^12 = 4096]
*/
void etm_set_cci(volatile ETM_interface *etm, int cci)
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


void etm_set_sync(volatile ETM_interface *etm, int p)
{
    etm->sync_period = p;
}



void etm_set_stall(volatile ETM_interface *etm, int level)
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

void etm_set_branch_broadcast(volatile ETM_interface *etm, int inv, uint8_t mask)
{
    SET(etm->trace_config, 3);
    if (inv)
        SET(etm->branch_broadcast_ctrl, 8);
    else
        CLEAR(etm->branch_broadcast_ctrl, 8);
    etm->branch_broadcast_ctrl |= mask;
}

static void etm_set_addr_cmp(volatile ETM_interface *etm, int num, uint64_t addr, int cmp_contextid)
{
    etm->addr_cmp_val[num] = addr ;
    if (cmp_contextid)
        SET(etm->addr_cmp_access_type[num], 2);
    else
        CLEAR(etm->addr_cmp_access_type[num], 2);
    CLEAR(etm->addr_cmp_access_type[num], 3);
}

void etm_register_range(volatile ETM_interface *etm, uint64_t start_addr, uint64_t end_addr, int cmp_contextid)
{
    int addr_cmp_index_base = _request_addr_cmp_pair(etm);
    etm_set_addr_cmp(etm, addr_cmp_index_base, start_addr, cmp_contextid);
    etm_set_addr_cmp(etm, addr_cmp_index_base + 1, end_addr, cmp_contextid);
    SET(etm->vi_ie_ctrl, addr_cmp_index_base / 2);
}

void etm_register_start_stop_addr(volatile ETM_interface *etm, uint64_t start_addr, uint64_t end_addr)
{
    int cmp_0 = _request_addr_cmp(etm);
    int cmp_1 = _request_addr_cmp(etm);
    etm_set_addr_cmp(etm, cmp_0, start_addr, 1);
    etm_set_addr_cmp(etm, cmp_1, end_addr, 1);
    etm->vi_main_ctrl = 0x1;
    SET(etm->vi_ss_ctrl, cmp_0);
    SET(etm->vi_ss_ctrl, cmp_1 + 16);
}



/*
    rs_num   : Resource Selector number
    rs_group : see header. For PMU event, use External_input
    r1       : According to the group, r1 represents the corresponding sub-resource number
    r2       : only Couter&Sequencer share the same group. Thus if the chosen group is not this, r2 would be ignored
               otherwise, r2 represents the Sequencer number. Explicitly r2=-1 if sequencer is not used
    inv      : Whether inverse the results
    pair_inv : Whether inverse the combined result from pair resources
*/
static void etm_set_rs(volatile ETM_interface *etm, int rs_num, enum rs_group group, int r1, int r2, int inv, int pair_inv)
{
    if (rs_num < 2) {
        printf("WARNING: Resource Selector 0,1 are special RS. Should not be used. RS not set.\n");
        return ;
    }
    if (group == Counter_Seq) {
        SET(etm->resource_sel_ctrl[rs_num], r1);
        if (r2 >= 0) {
            SET(etm->resource_sel_ctrl[rs_num], r2 + 4);
        }
    } else
        SET(etm->resource_sel_ctrl[rs_num], r1);
    etm->resource_sel_ctrl[rs_num] |= group << 16 ;
    if (inv)
        SET(etm->resource_sel_ctrl[rs_num], 20);
    if (pair_inv)
        SET(etm->resource_sel_ctrl[rs_num], 21);
}

static void etm_set_event_sel_n(volatile ETM_interface *etm, int rs_num, int pair, int n)
{
    etm->event_ctrl_0 |= rs_num << (8*n);
    if (pair)
        SET(etm->event_ctrl_0, 7 + 8*n);
    else
        CLEAR(etm->event_ctrl_0, 7 + 8*n);
}

/*
    Hook the resource indicated by [rs_num] and [pair] to the ETM event at position [sel_num]
*/
static void etm_set_event_sel(volatile ETM_interface *etm, int sel_num, int rs_num, int pair)
{
    if(!pair && rs_num < 2) {
        printf("WARNING: Resource Selector 0,1 are special RS, and is used. Make sure it's intended.\n");
    }
    int true_num = rs_num;
    if (pair) {
        true_num = rs_num / 2;
    }
    assert(sel_num < 4 && sel_num >= 0);
    etm_set_event_sel_n(etm, true_num, pair, sel_num);
}


void etm_set_event_trc(volatile ETM_interface *etm, int mask, int atb)
{
    etm->event_ctrl_1 |= mask;
    if (atb)
        SET(etm->event_ctrl_1, 11);
    else
        CLEAR(etm->event_ctrl_1, 11);
}


void etm_always_fire_event_pos(volatile ETM_interface *etm, int pos)
{
    etm_set_event_sel(etm, pos, 1, 0);
    etm_set_event_trc(etm, 0x1 << pos, 0);
}

void etm_register_pmu_event(volatile ETM_interface *etm, int event_bus)
{
    int rs_num = _request_rs(etm);
    int ext_num = _request_ext_sel(etm);

    etm_set_ext_input(etm, event_bus, ext_num);
    etm_set_rs(etm, rs_num, External_input, ext_num, 0, 0, 0);
    etm_set_event_sel(etm, ext_num, rs_num, 0);

    // int trc_mask = etm->event_ctrl_1 & 0xf ;
    // trc_mask |= 0x1 << ext_num ;
    // etm_set_event_trc(etm, trc_mask, 0);
    etm_set_event_trc(etm, 0x1 << ext_num, 0);
}

uint8_t etm_prepare_external_input_resource(volatile ETM_interface *etm, int event_bus)
{
    int rs_num = _request_rs(etm);
    int ext_num = _request_ext_sel(etm);
    etm_set_ext_input(etm, event_bus, ext_num);
    etm_set_rs(etm, rs_num, External_input, ext_num, -1, 0, 0);
    return rs_num;
}

uint8_t etm_prepare_short_counter(volatile ETM_interface* etm, uint16_t counter_val, uint8_t rs_num)
{
    uint8_t ct_num = _request_ct(etm);
    etm->counter_ctrl[ct_num] = rs_num;  // when rs_num fires, counter decrement

    // set initial/reload value
    etm->counter_val[ct_num] = counter_val;
    etm->counter_ctrl[ct_num] |= 0x1 << 16;    // self-reload
    etm->counter_reload_val[ct_num] = counter_val;    // reload value

    return ct_num;
}

uint8_t etm_prepare_counter_fire_resource(volatile ETM_interface* etm, uint8_t counter_num)
{
    uint8_t rs_num = _request_rs(etm);
    etm_set_rs(etm, rs_num, Counter_Seq, counter_num, -1, 0, 0);
    return rs_num;
}

void etm_event_for_resource(volatile ETM_interface* etm, uint8_t event_pos, uint8_t rs_num)
{
    etm_set_event_sel(etm, event_pos, rs_num, 0);
    etm_set_event_trc(etm, 0x1 << event_pos, 0);
}

void etm_example_short_counter_fire_event(volatile ETM_interface* etm, int event_bus, uint16_t counter_val, uint8_t event_position)
{
    printf("Running example: Single counter counting Event Bus %d with reload %u and fire Event\n", event_bus, counter_val);
    printf("Event position: %d\n", event_position);

    uint8_t rs_for_ext_input = etm_prepare_external_input_resource(etm, event_bus);
    uint8_t ct_num = etm_prepare_short_counter(etm, counter_val, rs_for_ext_input);
    uint8_t rs_for_counter = etm_prepare_counter_fire_resource(etm, ct_num);
    etm_event_for_resource(etm, event_position, rs_for_counter);

}



void etm_set_large_counter(volatile ETM_interface* etm, int cnt_base_index, uint32_t val)
{
    // when forming larger counter by using two counters, the cnt_base_index should be even. On Cortex-A53, only two cnts are available
    // thus the only valid value is 0
    assert(cnt_base_index == 0);
    etm->counter_val[cnt_base_index] = val;
    etm->counter_val[cnt_base_index + 1] = val >> 16;
    etm->counter_reload_val[cnt_base_index] = val;
    etm->counter_reload_val[cnt_base_index + 1] = val >> 16;

    etm->counter_ctrl[cnt_base_index] |= 0x1 << 16; // self-reload
    etm->counter_ctrl[cnt_base_index + 1] |= 0x1 << 16; // self-reload
    etm->counter_ctrl[cnt_base_index + 1] |= 0x1 << 17; // forming a larger counter
}

void etm_print_large_counter(volatile ETM_interface* etm, int cnt_base_index)
{
    printf("%10d\n", etm->counter_val[cnt_base_index] | (etm->counter_val[cnt_base_index + 1] << 16));
}

void etm_example_large_counter(volatile ETM_interface* etm, int event_bus, uint32_t counter_val)
{
    printf("Large counter counting Evnet Bus %d\n", event_bus);
    printf("Reload value: %d\n", counter_val);
    printf("IMPORTANT: read counter value when ETM is active might return unstable value!\n");

    int rs_num = _request_rs(etm);

    // when event indicated by resource [rs_num] occurs, counter 0 is decremented
    etm->counter_ctrl[0] = rs_num;

    // request a external input selector
    int ext_num = _request_ext_sel(etm);

    // let the resource rs_num hooked to the external input selector when PMU fires event_bus
    etm_set_rs(etm, rs_num, External_input, ext_num, -1, 0, 0);
    etm_set_ext_input(etm, event_bus, ext_num);

    etm_set_large_counter(etm, 0, counter_val);
}

void etm_example_large_counter_fire_event(volatile ETM_interface* etm, int event_bus, uint32_t counter_val)
{
    printf("Running example: Large counter counting Event Bus and fire Event\n");
    printf("IMPORTANT: read counter value when ETM is active might return unstable value!\n");
    // We need three resource regs to make this work
    // one for monitoring the PMU event bus
    // two for forming the logic to use the large counter
    int rs_pmu_bus = _request_rs(etm);
    int rs_pair_base = _request_rs_pair(etm);

    // to monitor PMU event bus, we also need a External Input Selector
    int ext_num = _request_ext_sel(etm);

    // let ext_num listen to the desired event bus
    etm_set_ext_input(etm, event_bus, ext_num);

    // then let the resource rs_pmu_bus listen to the ext_num
    etm_set_rs(etm, rs_pmu_bus, External_input, ext_num, -1, 0, 0);

    // forming the large counter
    etm_set_large_counter(etm, 0, counter_val);

    // the lower part of the counter decrements when rs_pmu_bus fires
    etm->counter_ctrl[0] |= rs_pmu_bus;

    // the resource pair should listen to the lower and upper counter
    // they use (A and B) logic, so the resource pair fire when both counter is zero
    etm_set_rs(etm, rs_pair_base, Counter_Seq, 0, -1, 0, 0);
    etm_set_rs(etm, rs_pair_base + 1, Counter_Seq, 1, -1, 0, 0);

    // finally we tell ETM to insert Event Packet when the resource pair fires
    // This also asserts ETM's own output pin for event occurance

    // the position is arbitrary, choose from [0...3]
    int position_in_event_packet = 3;

    // let ETM assert Event when the resource fires, this produces external output to CTI
    etm_set_event_sel(etm, position_in_event_packet, rs_pair_base, 1);

    // let ETM further insert Event Packet when the resource fires
    etm_set_event_trc(etm, 0x1 << position_in_event_packet, 0);

}


void etm_example_large_counter_rapid_fire_pos(volatile ETM_interface* etm, int pos, uint32_t counter_val)
{
    int rs_pair = _request_rs_pair(etm);

    etm_set_large_counter(etm, 0, counter_val);

    etm->counter_ctrl[0] |= 1;

    etm_set_rs(etm, rs_pair, Counter_Seq, 0, -1, 0, 0);
    etm_set_rs(etm, rs_pair + 1, Counter_Seq, 1, -1, 0, 0);

    etm_set_event_sel(etm, pos, rs_pair, 1);
    etm_set_event_trc(etm, 0x1 << pos, 0);
}

void etm_register_single_addr_match_event(volatile ETM_interface *etm, uint64_t addr)
{
    int addr_cmp_num = _request_addr_cmp(etm);
    int rs_num = _request_rs(etm);
    int ext_num = _request_ext_sel(etm);

    etm_set_addr_cmp(etm, addr_cmp_num, addr, 1);
    etm_set_rs(etm, rs_num, Single_addr, addr_cmp_num, 0,0,0);
    etm_set_event_sel(etm, ext_num, rs_num, 0);
    etm_set_event_trc(etm, 0x1 << ext_num, 0);

}
