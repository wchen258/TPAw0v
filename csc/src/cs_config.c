#include <stdio.h>
#include <stdlib.h>
#include "cs_etm.h"
#include "cs_pmu.h"
#include "cs_soc.h"
#include "zcu_cs.h"

volatile ETM_interface *etms[4] = {NULL, NULL, NULL, NULL};
volatile Replicator_interface *replicator = NULL;
volatile Funnel_interface *funnel1 = NULL;
volatile Funnel_interface *funnel2 = NULL;
volatile TMC_interface *tmc1 = NULL;
volatile TMC_interface *tmc2 = NULL;
volatile TMC_interface *tmc3 = NULL;
volatile TPIU_interface *tpiu = NULL;

volatile CTI_interface *r0_cti = NULL;
volatile CTI_interface *r1_cti = NULL;
volatile CTI_interface *a0_cti = NULL;
volatile CTI_interface *a1_cti = NULL;
volatile CTI_interface *a2_cti = NULL;
volatile CTI_interface *a3_cti = NULL;

volatile CTI_interface *cti0 = NULL;
volatile CTI_interface *cti1 = NULL;
volatile CTI_interface *cti2 = NULL;

volatile PMU_interface *pmus[4] = {NULL, NULL, NULL, NULL};


void cs_config_tmc1_softfifo() {
	printf("Trace data path: software directly polls TMC1 (aka ETF1).\n\n");

	etms[0] = (volatile ETM_interface *) cs_register(A53_0_etm);
	etms[1] = (volatile ETM_interface *) cs_register(A53_1_etm);
	etms[2] = (volatile ETM_interface *) cs_register(A53_2_etm);
	etms[3] = (volatile ETM_interface *) cs_register(A53_3_etm);
    funnel1 = (volatile Funnel_interface *) cs_register(Funnel1);
    funnel2 = (volatile Funnel_interface *) cs_register(Funnel2);
    tmc1 = (volatile TMC_interface *) cs_register(Tmc1);

	funnel_unlock(funnel1);
	funnel_unlock(funnel2);

	// enabling all the slaves ports on the funnel
	// it seems at least one needs to be enabled to make the funnel work
	funnel_config_port(funnel1, 0xff, 0);
	funnel_config_port(funnel2, 0xff, 0);

	munmap((void *)funnel1, sizeof(Funnel_interface));
	munmap((void *)funnel2, sizeof(Funnel_interface));

	tmc_unlock(tmc1);
	tmc_disable(tmc1);
	tmc1->formatter_flush_ctrl = 0x3;  // enable formatter and trigger, trigger is not used though in this config
	tmc_set_mode(tmc1, Soft);

	// if TMC1 does use AXI, it will have the widest system access
	tmc_set_axi(tmc1, 0xf);

	tmc_enable(tmc1);
	return ;
}


void cs_config_SRAM() {
	printf("Trace data path: TMC1 -> TMC2 -> SRAM.\n\n");

	etms[0] = (volatile ETM_interface *) cs_register(A53_0_etm);
	etms[1] = (volatile ETM_interface *) cs_register(A53_1_etm);
	etms[2] = (volatile ETM_interface *) cs_register(A53_2_etm);
	etms[3] = (volatile ETM_interface *) cs_register(A53_3_etm);
    funnel1 = (volatile Funnel_interface *) cs_register(Funnel1);
    funnel2 = (volatile Funnel_interface *) cs_register(Funnel2);
    tmc1 = (volatile TMC_interface *) cs_register(Tmc1);
    tmc2 = (volatile TMC_interface *) cs_register(Tmc2);

	funnel_unlock(funnel1);
	funnel_unlock(funnel2);
	funnel_config_port(funnel1, 0xff, 0);
	funnel_config_port(funnel2, 0xff, 0);

	tmc_unlock(tmc1);
	tmc_unlock(tmc2);

	tmc_disable(tmc1);
	tmc_disable(tmc2);

	tmc_set_mode(tmc1, Hard);
	tmc_set_mode(tmc2, Circular);

	// enable formatter and trigger, trigger is not used though in this config
	// whenever more than one core is being traced. The formatter is a must
	tmc1->formatter_flush_ctrl = 0x3;
	tmc2->formatter_flush_ctrl = 0x3;

	tmc_set_axi(tmc1, 0xf);
	tmc_set_axi(tmc2, 0xf);

	tmc_enable(tmc1);
	tmc_enable(tmc2);

	munmap((void *)tmc1, sizeof(TMC_interface));

	return ;
}


void cs_config_etr_mp(uint64_t buf_addr, uint32_t buf_size) {
	printf("Trace data path: TMC1(HardFIFO) -> TMC2(HardFIFO) -> TMC3(ETR in Circular) -> 0x%lx\n", buf_addr);
	printf("ETR assumes the buffer size is %d bytes\n", buf_size);

	etms[0] = (volatile ETM_interface *) cs_register(A53_0_etm);
	// etms[1] = (volatile ETM_interface *) cs_register(A53_1_etm);
	// etms[2] = (volatile ETM_interface *) cs_register(A53_2_etm);
	// etms[3] = (volatile ETM_interface *) cs_register(A53_3_etm);
    replicator = (volatile Replicator_interface *) cs_register(Replic);
    funnel1 = (volatile Funnel_interface *) cs_register(Funnel1);
    funnel2 = (volatile Funnel_interface *) cs_register(Funnel2);
    tmc1 = (volatile TMC_interface *) cs_register(Tmc1);
    tmc2 = (volatile TMC_interface *) cs_register(Tmc2);
    tmc3 = (volatile TMC_interface *) cs_register(Tmc3);


	// We are not using TPIU, so let Replicator discard all transactions directing to TPIU
	// on ZCU102, TPIU is not powered by default. To power, connect jumper J88 on board
	replicator->lock_access = 0xc5acce55;
	replicator->id_filter_atb_master_p_1 = 0xff;
	munmap((void *)replicator, sizeof(Replicator_interface));

	funnel_unlock(funnel1);
	funnel_unlock(funnel2);
	funnel_config_port(funnel1, 0xff, 0);
	funnel_config_port(funnel2, 0xff, 0);
	munmap((void *)funnel1, sizeof(Funnel_interface));
	munmap((void *)funnel2, sizeof(Funnel_interface));

	tmc_unlock(tmc1);
	tmc_unlock(tmc2);
	tmc_unlock(tmc3);

	tmc_disable(tmc1);
	tmc_disable(tmc2);
	tmc_disable(tmc3);

	tmc_set_mode(tmc1, Hard);
	tmc_set_mode(tmc2, Hard);
	tmc_set_mode(tmc3, Circular);

	// enable formatter and trigger, trigger is not used though in this config
	// whenever more than one cores are being traced. The formatter is a must
	tmc1->formatter_flush_ctrl = 0x3;
	tmc2->formatter_flush_ctrl = 0x3;
	tmc3->formatter_flush_ctrl = 0x3;

	tmc1->buf_level_water_mark = 0x0;
	tmc2->buf_level_water_mark = 0x0;

	tmc_set_axi(tmc3, 0xf);

	// ETR specific configuration
	tmc_set_size(tmc3, buf_size); // this function would divide the buf_size by 4 to write to the register in unit of word
	tmc_set_data_buf(tmc3, buf_addr);
	tmc_set_read_pt(tmc3, buf_addr);
	tmc_set_write_pt(tmc3, buf_addr);

	tmc_enable(tmc1);
	tmc_enable(tmc2);
	tmc_enable(tmc3);

	// munmap((void *)tmc1, sizeof(TMC_interface));
	// munmap((void *)tmc2, sizeof(TMC_interface));
	// munmap((void *)tmc3, sizeof(TMC_interface));

	return ;
}


void config_pmu_enable_export()
{
	printf("Configuring PMU to allow exporting architectural event to ETM\n");
	pmus[0] = (volatile PMU_interface *) cs_register(A53_0_pmu);
	pmus[1] = (volatile PMU_interface *) cs_register(A53_1_pmu);
	pmus[2] = (volatile PMU_interface *) cs_register(A53_2_pmu);
	pmus[3] = (volatile PMU_interface *) cs_register(A53_3_pmu);

	for(int i = 0; i < 4; i++)
	{
		pmus[i]->lock_access = 0xc5acce55;
		pmus[i]->ctrl = 0x11;

		munmap((void *)pmus[i], sizeof(PMU_interface));
	}


}

void config_etm_n(volatile ETM_interface* etm_n, int stall, int id)
{
	etm_unlock(etm_n);
	etm_disable(etm_n);
	etm_reset(etm_n); // reset would set id to 1
	etm_n->trace_id = id; // so assign a none conflict id
	etm_set_cid(etm_n); // set_cid cause the ETM to track context ID
	etm_set_stall(etm_n, stall);
	etm_set_sync(etm_n, 0);

}

void cti_setup()
{
    cti_config(a0_cti, 0x8);
    cti_config(a1_cti, 0x8);
    cti_config(a2_cti, 0x8);
    cti_config(a3_cti, 0x8);
    cti_config(cti0, 0x8);
    // cti_config(cti1);
    // cti_config(cti2);
    cti_config(r0_cti, 0x0);

    for (int i = 0; i < 4; ++i)
    {
        a0_cti->trig_to_channel_en[4 + i] = 0b1000;
        a1_cti->trig_to_channel_en[4 + i] = 0b1000;
        a2_cti->trig_to_channel_en[4 + i] = 0b1000;
        a3_cti->trig_to_channel_en[4 + i] = 0b1000;
    }

    cti0->channel_to_trig_en[0] = 0b1000;

    // connect A53 CTI's trigout to enter/leave debug mode
    // channel 0b0100 for entering A530 debug, 0b0010 for resuming
    a0_cti->channel_to_trig_en[0] = 0b0100;
    a0_cti->channel_to_trig_en[1] = 0b0010;
    a1_cti->channel_to_trig_en[0] = 0b0100;
    a1_cti->channel_to_trig_en[1] = 0b0010;
    a2_cti->channel_to_trig_en[0] = 0b0100;
    a2_cti->channel_to_trig_en[1] = 0b0010;
    a3_cti->channel_to_trig_en[0] = 0b0100;
    a3_cti->channel_to_trig_en[1] = 0b0010;

}
