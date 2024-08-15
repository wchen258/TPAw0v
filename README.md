# TPAw0v
The implementation presented in [Timely Progress Integrity: Low-overhead Online Assessment of Timely Progress as a Commodity](https://drops.dagstuhl.de/entities/document/10.4230/LIPIcs.ECRTS.2023.13)

### Recommanded: Start with the csc directory
The program `start_mp` in `csc`, upon executing, configures the infrastructure necessary, runs a target program `./hello_ETM`, traces the target program, and prints out the trace data upon exiting. 

**Analyze Trace Data**
`start_mp` should output two files `trace.dat` and `trace.out`. Then use the `deformat` to parser the formatted trace data by `./deformat 1 trace.dat`. The `1` indicates only one ETM is used during the trace session. This should output the raw trace data `trc_0.out`. Then use the `ctrace` in `ETM_data_parser` by `./ctrace trc_0.out`. It should print human readable trace data. 

**TL;DR**
The purpose of `csc` is to provide a starting point for researches who want to dive deep into the CoreSight trace infrastructure. The code has been refactored a number of times for simplicity. `csc` does not entirely replicate the configuration presented in the paper. However, it has already performed one of the important aspect: set up the CoreSight infrastructure, and perform a toy example in tracing. It is intended to serve as a Hello World! for CoreSight ETM trace on ZCU102 or Kria board. The program in `csc` assumes the execution environment is some Linux on ZCU102 or Kria boards. 

What does the program `start_mp` in `csc` do in details? It first configures CoreSight components, so that they are ready to accept trace data from ETM. Most importantly, it only uses one Trace Memory Controller (TMC) *TMC1* (a.k.a *ETF1*) in Software FIFO mode. This is different from paper implementation in which all three *TMCx* are used to route the trace data to some memory storage. The `poller` as a piece of software can run anywhere on the platform and it constantly polls the RAM Read Data register on TMC to fetch trace data while the target program is running. You can even add flush logic to the `poller` so that the data is fetched in a real-time manner. The `poller` can be placed on other Processing Element (PE), e.g. other spare Cortex-A53, Cortex-R5, or FPGA. Then `start_mp` enables *ETM* to generate trace while fork the target program `./hello_ETM` to be traced. Specifically, it sets a filter so that only the CPU activity for the target program within virtual address `0x400000 - 0x500000` will be traced. When the target program is finished, `start_mp` will disable the *ETM* and instruct `poller` to print out the trace data. 

### Kernel Configuration ###
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


