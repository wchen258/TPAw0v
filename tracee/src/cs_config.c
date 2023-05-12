#include <stdio.h>
#include <stdlib.h>
#include "cs_etm.h"
#include "cs_soc.h"
#include "zcu_cs.h"

ETM_interface *etm;
ETM_interface *etms[4];
Replicator_interface *replicator;
Funnel_interface *funnel1;
Funnel_interface *funnel2;
TMC_interface *tmc1;
TMC_interface *tmc2;
TMC_interface *tmc3;

void cs_config_etr(uint64_t buf_addr, uint32_t buf_size) {
	printf("    ------Coresight Configure Using ETR------\n");
	printf(" Buffer Address: 0x%lx\n", buf_addr);
	printf(" Buffer Size   : %d (bytes)\n",  buf_size);
	printf("    -------------- End Info -----------------\n\n");

	etm = (ETM_interface *) cs_register(A53_0_etm);
    replicator = (Replicator_interface *) cs_register(Replic);
    funnel1 = (Funnel_interface *) cs_register(Funnel1);
    funnel2 = (Funnel_interface *) cs_register(Funnel2);
    tmc1 = (TMC_interface *) cs_register(Tmc1);
    tmc2 = (TMC_interface *) cs_register(Tmc2);
    tmc3 = (TMC_interface *) cs_register(Tmc3);

	funnel_unlock(funnel1);
	funnel_unlock(funnel2);
	//funnel_config_port(funnel1, 0xf, 0);
	//funnel_config_port(funnel2, 0x4, 0); //0x4 is Lauterbach setting, for mp, try use all port
	funnel_config_port(funnel1, 0xff, 0);
	funnel_config_port(funnel2, 0xff, 0);

	tmc_unlock(tmc1);
	tmc_unlock(tmc2);
	tmc_unlock(tmc3);
	tmc_disable(tmc1);
	tmc_disable(tmc2);
	tmc_disable(tmc3);
	tmc_set_mode(tmc1, Hard);
	tmc_set_mode(tmc2, Hard);
	tmc_set_mode(tmc3, Circular);



	tmc_set_axi(tmc3, 0xf);
	tmc_set_size(tmc3, buf_size);
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
	printf("    ------Coresight Configure Using ETR------\n");
	printf(" Buffer Address: 0x%lx\n", buf_addr);
	printf(" Buffer Size   : %d (bytes)\n",  buf_size);
	printf("    -------------- End Info -----------------\n\n");

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
	tmc1->formatter_flush_ctrl = 0x1;
	tmc2->formatter_flush_ctrl = 0x1;
	tmc3->formatter_flush_ctrl = 0x1;
	tmc_set_axi(tmc3, 0xf);
	tmc_set_size(tmc3, buf_size);
	tmc_set_data_buf(tmc3, buf_addr);
	tmc_set_read_pt(tmc3, buf_addr);
	tmc_set_write_pt(tmc3, buf_addr);

	tmc_enable(tmc1);
	tmc_enable(tmc2);
	tmc_enable(tmc3);

	return ;
}
/*
	a generic etm config, most functions are disabled, non-invasive
*/
void config_etm() 
{
	etm_unlock(etm);
	etm_disable(etm);
	etm_reset(etm);
	etm_set_cid(etm);
	etm_set_stall(etm, 0);
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
	etm_set_cid(etm_n);
	etm_set_stall(etm_n, stall);
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
