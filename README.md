# TPAw0v; CoreSight ETM setup
The purpose of this repo is two-folds.

First, it contains a theoretical-minimum setup to conduct ARM CoreSight ETM trace on ZCU102/Kria board (`csc`, `deformat`, `ETM_data_parser`). Second, it contains paritial implementation presented in [Timely Progress Integrity: Low-overhead Online Assessment of Timely Progress as a Commodity](https://drops.dagstuhl.de/entities/document/10.4230/LIPIcs.ECRTS.2023.13) (`paper_imp`).

## Hello ETM! Set up a Trace Session

The code `csc` is to provide a starting point for researches who want to dive deep into the CoreSight trace infrastructure. The code has been refactored a number of times for simplicity. The program in `csc` assumes the execution environment is some Linux on ZCU102 or Kria boards (or boards with same CoreSight Topology). It performs setup for necessary CoreSight components, including but not limited to Embedded Trace Macrocell (ETM). 

### Demo I: Send trace data to any memory-mapped address
`start_etr` in `csc` offers an example setup that utilizes the Embedded Trace Router (ETR) in Circular Buffer Mode to rout trace data to any memory-mapped storage. Just go to `csc` directory, and `make`. Then `./start_etr`. The resultant trace data `trace.dat` can be analyzed by `deformat`, e.g. `./deformat 1 trace.dat` which produces `trc_0.out`, then by `ETM_data_parser/ctrace`, e.g. `./ctrace trc_0.out`. If something goes wrong, take a look at Kernel Configuration in the later section.

In `start_etr.c`, you can specify the memory address, size, and the target application to trace. The trace data paths are Advanced Trace Bus (ATB) and AXI. Thus it expects to have higher throughput. Notice, by default, CoreSight does not guarantee real-time data emission. To achieve real-time, you need to use flush function offered by ETR. 

### Demo II: Poll the trace data by software
The program `start_mp` in `csc`, upon executing, configures the infrastructure necessary, runs a target program `./hello_ETM`, traces the target program, and prints out the trace data upon exiting. The trace data is acquired by a child process `poller` which constantly polls the RAM Read Data register on the second Trace Memory Controller (TMC2).

**TL;DR**

What does the program `start_mp` in `csc` do in details? It first configures CoreSight components, so that they are ready to accept trace data from ETM. Most importantly, it only uses one Trace Memory Controller (TMC) *TMC1* (a.k.a *ETF1*) in Software FIFO mode. This is different from paper implementation in which all three *TMCx* are used to route the trace data to some memory storage. The `poller` as a piece of software can run anywhere on the platform and it constantly polls the RAM Read Data register on TMC to fetch trace data while the target program is running. You can even add flush logic to the `poller` so that the data is fetched in a real-time manner. The `poller` can be placed on other Processing Element (PE), e.g. other spare Cortex-A53, Cortex-R5, or FPGA. Then `start_mp` enables *ETM* to generate trace while fork the target program `./hello_ETM` to be traced. Specifically, it sets a filter so that only the CPU activity for the target program within virtual address `0x400000 - 0x500000` will be traced. When the target program is finished, `start_mp` will disable the *ETM* and instruct `poller` to print out the trace data. 

The trace data path involves ATB and Advanced Peripheral Bus (APB), thus expects to be slower.

### Demo III: Use TMC local buffer
`start_sram` in `csc` demonstrates how to use TMC2 in Circular Buffer Mode to direct store trace data on the buffer managed by TMC2. This approach offers the least hackability, because the address of the trace data is not visible to the user, and the data has to be read via APB. The advantage of this approaches are: 1. no additional software (e.g. `poller`) is required. 2. non-intruisive (it does not impact the system performance. Although all Demos have very small impact, this is expected to be the least). 3. no extra memory storage is needed. 


## Advanced Trace Feature
ETM can interact with other CoreSight components such as Cross Trigger Interface (CTI) and Performance Monitor Unit (PMU)

### ETM inserting an Event Packet upon Input from PMU
For example, when a L2 cache miss occurs, PMU can send signal to ETM, and then ETM will indicates such an event as Event Packet into the trace data.
`start_etm_pmu` illustrates such usage. If it does not work out-of-box, you might need to insert the kernel module provided in `support/enable_arm_pmu.c`. 

### ETM inserting an Event Packet upon a user-defined number of Inputs from PMU
Continuing the example above, `start_cnt_pmu_event` fires an Event Packet to trace stream, for every **user-defined-number** L2 cache miss indicated by PMU. When monitoring a rather frequent signal from PMU, this is the recommanded approach. Because if we allow one Event Packet to be generated for each signal, overflow can occur for ETM. 

See `README.md` in `csc` for details using advanced features. 

## Kernel Configuration ###
The Linux on target has to be compile with certain flags to fully support trace. One [Lauterbach manual](https://www2.lauterbach.com/pdf/training_rtos_linux.pdf) details the configuration. Below is the summary.

```
cat /proc/config.gz | gunzip
```
The command above can list the current running kernel configuration. This command works if `CONFIG_IKCONFIG_PROC` is set during the kernel compilation (which is usually yes). 

During the kerneil compilation, the following options should be disabled

```
CONFIG_DEBUG_INFO_REDUCED
CONFIG_DEBUG_INFO_SPLIT
CONFIG_RANDOMIZE_BASE
CONFIG_SOFTLOCKUP_DETECTOR
CONFIG_DETECT_HUNG_TASK
CONFIG_CPU_IDLE
CONFIG_CPU_FREQ
CONFIG_UNMAP_KERNEL_AT_EL0
```
Notice `CONFIG_UNMAP_KERNEL_AT_EL0` is not available prior to kernel version 4.16. `CONFIG_SOFTLOCKUP_DETECTOR` and `CONFIG_DETECT_HUNG_TASK` are recommended to be disabled, might not be mandatory.
The following options should be set
```
CONFIG_KALLSYMS=y
CONFIG_DEBUG_INFO=y
```
It's also recommended enable the following option to better work with trace.
```
CONFIG_PID_IN_CONTEXTIDR=y
```

The `CONFIG_CPU_IDLE` and `CONFIG_CPU_FREQ` might be set. In the manual, the following instructions could be referenced without recompile the kernel.

> The Linux kernel CPU power management could cause for some processor architectures that single cores are not accessible by the debugger when in power saving state. CPU power management can be disabled in the Linux kernel configuration by disabling the options `CONFIG_CPU_IDLE` and` CONFIG_CPU_FREQ`.Idle states can also be disabled for single cores from the shell by writing to the file `/sys/devices/system/cpu/cpu<x>/cpuidle/state<x>/disable`. 

> Alternatively, you may remove the idle-states property from the device tree if available.On some Linux distributions, power management can be disabled using specific kernel command line parameters (e.g. “`jtag=on`” or “`nohlt`”). Please refer to the documentation of the kernel command line parameters of your Linux distribution for more information. 

So do something like
```
echo 1 | tee $(ls /sys/devices/system/cpu/cpu?/cpuidle/state?/disable) 2> /dev/null
```
On zcu quad core dev board, this should write to 8 files in total (each CPU has 2 idle states: state0 and state1).


