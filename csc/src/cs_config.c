#include <stdio.h>
#include <stdlib.h>
#include "cs_etm.h"
#include "cs_soc.h"
#include "zcu_cs.h"

ETM_interface *etms[4];
Replicator_interface *replicator;
Funnel_interface *funnel1;
Funnel_interface *funnel2;
TMC_interface *tmc1;
TMC_interface *tmc2;
TMC_interface *tmc3;

CTI_interface *r0_cti;
CTI_interface *r1_cti;
CTI_interface *a0_cti;
CTI_interface *a1_cti;
CTI_interface *a2_cti;
CTI_interface *a3_cti;

CTI_interface *cti0;
CTI_interface *cti1;
CTI_interface *cti2;


void cs_config_tmc1_softfifo() {
	printf("Trace data path: software directly polls TMC1 (aka ETF1).\n\n");

	etms[0] = (ETM_interface *) cs_register(A53_0_etm);
    replicator = (Replicator_interface *) cs_register(Replic);
    funnel1 = (Funnel_interface *) cs_register(Funnel1);
    funnel2 = (Funnel_interface *) cs_register(Funnel2);
    tmc1 = (TMC_interface *) cs_register(Tmc1);

	funnel_unlock(funnel1);
	funnel_unlock(funnel2);
	funnel_config_port(funnel1, 0xff, 0);
	funnel_config_port(funnel2, 0xff, 0);

	tmc_unlock(tmc1);
	tmc_disable(tmc1);
	tmc1->formatter_flush_ctrl = 0x3;  // enable formatter and trigger, trigger is not used though in this config
	tmc_set_mode(tmc1, Soft);
	tmc_set_axi(tmc1, 0xf);
	tmc_enable(tmc1);
	return ;	
}


void cs_config_etr_old(uint64_t buf_addr, uint32_t buf_size) {
	printf("    ------Coresight Configure Using ETR------\n");
	printf(" Buffer Address: 0x%lx\n", buf_addr);
	printf(" Buffer Size   : %d (bytes)\n",  buf_size);
	printf("    -------------- End Info -----------------\n\n");

	etms[0] = (ETM_interface *) cs_register(A53_0_etm);
    replicator = (Replicator_interface *) cs_register(Replic);
    funnel1 = (Funnel_interface *) cs_register(Funnel1);
    funnel2 = (Funnel_interface *) cs_register(Funnel2);
    tmc1 = (TMC_interface *) cs_register(Tmc1);
    tmc2 = (TMC_interface *) cs_register(Tmc2);
    tmc3 = (TMC_interface *) cs_register(Tmc3);

	funnel_unlock(funnel1);
	funnel_unlock(funnel2);
	// funnel_config_port(funnel1, 0xf, 0);
	// funnel_config_port(funnel2, 0x4, 0);
	funnel_config_port(funnel1, 0xff, 0);
	funnel_config_port(funnel2, 0xff, 0);

	tmc_unlock(tmc1);
	tmc_unlock(tmc2);
	tmc_unlock(tmc3);
	tmc_disable(tmc1);
	tmc_disable(tmc2);
	tmc_disable(tmc3);
	tmc1->formatter_flush_ctrl = 0x3;
	tmc_set_mode(tmc1, Soft);
	tmc_set_mode(tmc2, Hard);
	tmc_set_mode(tmc3, Circular);
	tmc_set_axi(tmc3, 0xf);
	tmc_set_axi(tmc2, 0xf);
	tmc_set_axi(tmc1, 0xf);
	tmc_set_size(tmc3, buf_size); // function would divide buf_size by 4 to write to register in unit of word
	tmc_set_data_buf(tmc3, buf_addr);
	tmc_set_read_pt(tmc3, buf_addr);
	tmc_set_write_pt(tmc3, buf_addr);

	tmc_enable(tmc1);
	tmc_enable(tmc2);
	tmc_enable(tmc3);

	return ;	
}

/*
	Registers and config necessary CS components for multiprocessor tracing
*/
void cs_config_etr_mp(uint64_t buf_addr, uint32_t buf_size) {

	etms[0] = (ETM_interface *) cs_register(A53_0_etm);
	etms[1] = (ETM_interface *) cs_register(A53_1_etm);
	etms[2] = (ETM_interface *) cs_register(A53_2_etm);
	etms[3] = (ETM_interface *) cs_register(A53_3_etm);
    replicator = (Replicator_interface *) cs_register(Replic);
    funnel1 = (Funnel_interface *) cs_register(Funnel1);
    funnel2 = (Funnel_interface *) cs_register(Funnel2);
    tmc1 = (TMC_interface *) cs_register(Tmc1);
    tmc2 = (TMC_interface *) cs_register(Tmc2);
    tmc3 = (TMC_interface *) cs_register(Tmc3);
	a0_cti = (CTI_interface *) cs_register(A53_0_cti);
	a1_cti = (CTI_interface *) cs_register(A53_1_cti);
	a2_cti = (CTI_interface *) cs_register(A53_2_cti);
	a3_cti = (CTI_interface *) cs_register(A53_3_cti);
	cti0 = (CTI_interface *) cs_register(Cti0);
	cti1 = (CTI_interface *) cs_register(Cti1);
	cti2 = (CTI_interface *) cs_register(Cti2);
	r0_cti = (CTI_interface *) cs_register(R5_0_cti);
	r1_cti = (CTI_interface *) cs_register(R5_1_cti);

	funnel_unlock(funnel1);
	funnel_unlock(funnel2);
	funnel_config_port(funnel1, 0xf, 0);
	funnel_config_port(funnel2, 0x4, 0);


	tmc_unlock(tmc1);
	tmc_unlock(tmc2);
	tmc_unlock(tmc3);

	tmc_disable(tmc1);
	tmc_disable(tmc2);
	tmc_disable(tmc3);

	tmc_set_mode(tmc1, Hard);
	tmc_set_mode(tmc2, Hard);
	tmc_set_mode(tmc3, Circular);

	// set the register for tmc1 in HARD FIFO mode
	// SET(tmc1->formatter_flush_ctrl, 0);
	// SET(tmc1->formatter_flush_ctrl, 1);
	// SET(tmc2->formatter_flush_ctrl, 0);
	// SET(tmc2->formatter_flush_ctrl, 1);

	// TrigOnTrigIn
	// SET(tmc3->formatter_flush_ctrl, 8);

	// // FOnTrigEvt
	// SET(tmc3->formatter_flush_ctrl, 5);

	// // StopOnFl
	// SET(tmc3->formatter_flush_ctrl, 12);

	// // EnTI
	// SET(tmc3->formatter_flush_ctrl, 1);

	// // EnFt
	// SET(tmc3->formatter_flush_ctrl, 0);
	

	tmc_set_axi(tmc1, 0xf);
	tmc_set_axi(tmc2, 0xf);
	tmc_set_axi(tmc3, 0xf);
	tmc_set_size(tmc3, buf_size); // this function would divide the buf_size by 4 to write to the register in unit of word
	tmc_set_data_buf(tmc3, buf_addr);
	tmc_set_read_pt(tmc3, buf_addr);
	tmc_set_write_pt(tmc3, buf_addr);
	// SET(tmc3->formatter_flush_ctrl, 0);
	// SET(tmc3->formatter_flush_ctrl, 1);

	tmc_enable(tmc1);
	tmc_enable(tmc2);
	tmc_enable(tmc3);

	return ;
}

/*
	stall = 0 for non-intrusive trace
*/
void config_etm_n(ETM_interface* etm_n, int stall, int id)
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

void config_etm_addr_event_test(ETM_interface *etm, uint64_t addr1, uint64_t addr2, uint64_t addr3, uint64_t addr4)
{
	etm_set_addr_cmp(etm, 0, addr1, 1);
	etm_set_addr_cmp(etm, 1, addr2, 1);
	etm_set_addr_cmp(etm, 2, addr3, 1);
	etm_set_addr_cmp(etm, 3, addr4, 1);

	etm_set_rs(etm, 2, Single_addr, 0,0,0,0);
	etm_set_rs(etm, 3, Single_addr, 1,0,0,0);
	etm_set_rs(etm, 4, Single_addr, 2,0,0,0);
	etm_set_rs(etm, 5, Single_addr, 3,0,0,0);

	etm_set_event_sel_0(etm, 2, 0);
	etm_set_event_sel_1(etm, 3, 0);
	etm_set_event_sel_2(etm, 4, 0);
	etm_set_event_sel_3(etm, 5, 0);

	etm_set_event_trc(etm, 0xf, 0);
}

void config_etm_single_pmu_event_test(ETM_interface *etm, int event_bus)
{

	// use External Input Selector 0 to event_but
	etm_set_ext_input(etm, event_bus, 0);

	// use Resource Selector 2 to monitor External Input Selector 0, thus the second zero is ignored, as the group is not Counter_Seq
	// last two zeros means not inverse the resutls
	etm_set_rs(etm, 2, External_input, 0, 0, 0, 0);

	// Use sel_0 position in trace stream to indicate the firing of Resource Selector 2, the ending 0 means not using the Resource as a pair
	etm_set_event_sel_0(etm, 2, 0);

	// enable the LSB in the bit mask to allow ETM insert event to trace stream
	// ending 0 means do not insert atb trigger if applicable
	etm_set_event_trc(etm, 1, 0);

}
