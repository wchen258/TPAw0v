/*
    Brief: adapted from start_etr.c, this demo also illustrates 
        how to emit Event Packet in trace stream when a user-chosen PMU event happens for a user-defined number of times.

    This demo should run on ZCU102/Kria board as long as the APU has linux running.

    Author: Weifan Chen
    Date: 2024-08-20
*/

/*
    Some observations:
        The address range can toggle the trace-on and trace-off.
        However, the emit of Event Packet is independent of the trace-on/trace-off state.
        When the event occurs, the ETM will send synchronization plus event packet. 

        This demo the poller does NOT attempt to flush the TMC at all.
        It's unclear whether the real-time property is preserved.
*/


#define _GNU_SOURCE
#include <stdio.h>
#include <sys/wait.h>
#include "common.h"
#include "pmu_event.h"
#include "cs_etm.h"
#include "cs_config.h"
#include "buffer.h"

extern ETM_interface *etms[4];

int main(int argc, char *argv[])
{
    printf("Vanilla ZCU102 self-host trace demo.\n");
    printf("Build: on %s at %s\n\n", __DATE__, __TIME__);

    pid_t target_pid; 

    // Disabling all cpuidle. Access the ETM of an idled core will cause a hang.
    linux_disable_cpuidle();
    
    // Pin to the 4-th core, because we will use 1st core to run the target application.  
    pin_to_core(3);

    uint64_t buf_addr = 0x00FFFC0000;  //OCM
    uint32_t buf_size = 1024 * 256;

    cs_config_etr_mp(buf_addr, buf_size);
    clear_buffer(buf_addr, buf_size);

    // enable PMU architectural event export
    config_pmu_enable_export();

    // initialize ETM
    config_etm_n(etms[0], 0, 1);

    // fork a child to execute the target application. 
    for (int i = 0; i < 1; i++)
    {
        target_pid = fork();
        if (target_pid == 0)
        {
            pin_to_core(i);
            uint64_t child_pid = (uint64_t) getpid();

            // further configure ETM. So that it will only trace the process with pid == child_pid/target_pid
            etm_set_contextid_cmp(etms[0], child_pid);
            etm_register_range(etms[0], 0x401144, 0x401144, 1); // this address is supposed to be the first instruction in <main>

            // choose one example to run
            //      example 1: use one counter (16-bit)
            // etm_example_single_counter_fire_event(etms[0], L2D_CACHE_REFILL_T, 65535); // 65535 is the max value for a 16-bit counter

            //      example 2: use two counters to form a 32 bit counter
            etm_example_large_counter_fire_event(etms[0], L2D_CACHE_REFILL_T, 100000); 

            //     example 3: test, use a large counter to see how fast it can emit event packet
            // etm_example_large_counter_rapid_fire_pos(etms[0], 0, 50000);

            // Enable ETM, start trace session
            etm_enable(etms[0]);

            // execute target application
            execl("./hello_ETM", "hello_ETM", NULL);
            perror("execl failed. Target application failed to start.");
            exit(1);
        }
        else if (target_pid < 0)
        {
            perror("fork");
            return 1;
        }
    }

    // wait for target application to finish
    int status;
    waitpid(target_pid, &status, 0);

    // Disable ETM, our trace session is done. Poller will print trace data.
    etm_disable(etms[0]);

    dump_buffer(buf_addr, buf_size);
    return 0;
}

