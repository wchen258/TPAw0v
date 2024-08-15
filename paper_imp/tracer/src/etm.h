#ifndef ETM_H
#define ETM_H

#include "zcu_cs.h"
#include "xil_printf.h"
#include "xtime_l.h"

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

// Address Comparator Value Register
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

#define MSG_BUFFER_SIZE (1024*2)
#define MS_LOG_SIZE (1500)
#define ETR_BUFFER_SIZE (1024*8)

typedef struct milestone_relay {
	uint8_t   n_valid;
	uint32_t  address[4];
	uint32_t  offset[4];
	uint32_t  nominal_t[4];
	uint32_t  tail_t[4];
} milestone_relay;

extern volatile uint32_t * milestones;
extern volatile uint32_t *milestone_type;
extern uint32_t milestones_size;
extern uint32_t current_milestone;
extern uint32_t current_timestamp;

void etm_disable();
void etm_enable();
void etr_disable();
void etr_enable();
void etr_man_flush();

void update_graph_milestone(uint32_t address);

#endif
