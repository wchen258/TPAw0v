#ifndef CS_CONFIG_H
#define CS_CONFIG_H

void cs_config_tmc1_softfifo();
void cs_config_etr_mp(uint64_t buf_addr, uint32_t buf_size);
void cs_config_SRAM();

/*  Apply a default configuration to ETM. 

    stall: the level of intrusiveness of the ETM.
    id:    the user defined id of the ETM.

    [stall] indicates how intrusive the ETM is allowed.
    the larger the [stall] the ETM more tends to stall the CPU 
    to mitigates the trace data buffer overflow.
    Set stall == 0, to disallow ETM to stall CPU. Ih this case
    when overflow occurs, an overflow packet would be inserted into 
    the trace stream 
    
    [id]: you can assign any value to [id] as the identifier for the ETM.
    However, some IDs are reserved. For example, 0x0 is reserved for padding.
    Check CoreSight Architecture Specification for more details.
    For the scope of this project, assign from 1 to n.
    
    For example, on ZCU102, assign 1 to the ETM of Cortex_A53_0, 2 to Cortex_A53_1, and so on.
*/
void config_etm_n(ETM_interface* etm_n, int stall, int id);

void config_etm_addr_event_test(ETM_interface*, uint64_t, uint64_t, uint64_t, uint64_t);
void config_etm_single_pmu_event_test(ETM_interface*, int event_bus_num);
void config_pmu_enable_export();

#endif