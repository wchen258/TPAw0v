#ifndef CS_CONFIG_H
#define CS_CONFIG_H

/*
	The following config instruct the TMC1 to be used as a software FIFO
	Other processing element (PE) can poll the TMC1 to read trace data
*/
void cs_config_tmc1_softfifo();


/*
	The configuration uses TMC3 (ETR) in circular buffer mode to stream the trace data
	to a user defined memory buffer.

    buf_addr: physical address of the buffer
    buf_size: size of the buffer in bytes

    Known issue:
        if the buffer is the on-chip memory (OCM) of ZCU102,
        the buffer need to be initialized to 0xffffffff before start tracing.
*/
void cs_config_etr_mp(uint64_t buf_addr, uint32_t buf_size);


/*
	The following config instructs TMC2 to store trace data to its dedicated SRAM
*/
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
void config_etm_n(volatile ETM_interface* etm_n, int stall, int id);


/*
	This function assume the PMU is writtable in user space.
	If something goes wrong,
	the support/enable_arm_pmu.c provides the kernel module to set PMU to be writtable in user space.
	Run the kernel module before running this function.

	In reality, the PMU should be accessible by default because we access it as if an external debugger..
*/
void config_pmu_enable_export();

#endif
