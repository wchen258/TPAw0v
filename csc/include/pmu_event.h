#ifndef PMU_EVENT_H
#define PMU_EVENT_H

// PMU EVENT and External Bus to ETM (post fix _T)

#define L1I_CACHE_REFILL    0x1
#define L1I_CACHE_REFILL_T  0
#define L1I_TLB_REFILL      0x2
#define L1I_TLB_REFILL_T    1
#define L1D_CACHE_REFILL    0x3
#define L1D_CACHE_REFILL_T  2
#define L1D_CACHE           0x4
#define L1D_CACHE_T         3
#define INST_RETIRED        0x8
#define INST_RETIRED_T      7
#define L1I_CACHE           0x14
#define L1I_CACHE_T         18
#define L1D_CACHE_WB        0x15
#define L1D_CACHE_WB_T      19
#define L2D_CACHE           0x16
#define L2D_CACHE_T         20
#define L2D_CACHE_REFILL    0x17
#define L2D_CACHE_REFILL_T  21
#define L2D_CACHE_WB        0x18
#define L2D_CACHE_WB_T      22
#define LD_RETIRED          0x6
#define LD_RETIRED_T        5
#define ST_RETIRED          0x7
#define ST_RETIRED_T        6
#define EXC_TAKEN           0x9
#define EXC_TAKEN_T         9
#define EXC_RETURN          0xa
#define EXC_RETURN_T        10
#define CID_WRITE_RETIRED   0xb
#define CID_WRITE_RETIRED_T 11
#define PC_WRITE_RETIRED    0xc
#define PC_WRITE_RETIRED_T  12
#define BR_IMMED_RETIRED    0xd
#define BR_IMMED_RETIRED_T  13
#define UNALIGNED_LDST_RETIRED   0xf
#define UNALIGNED_LDST_RETIRED_T 14
#define BR_MIS_PRED         0x10
#define BR_MIS_PRED_T       15
#define BR_PRED             0x12
#define BR_PRED_T           16
#define MEM_ACCESS          0x13
#define MEM_ACCESS_T        17
#define L1I_CACHE_ERR       0xd0
#define L1I_CACHE_ERR_T     23
#define L1D_CACHE_ERR       0xd1
#define L1D_CACHE_ERR_T     24
#define TLB_MEM_ERR         0xd2
#define TLB_MEM_ERR_T       25


#endif