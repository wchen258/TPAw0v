/*
    Brief: adapted from start_mp.c, this demo also illustrates how to insert PMU event into trace data.

    This demo should run on ZCU102/Kria board as long as the APU has linux running.

    Author: Weifan Chen
    Date: 2024-08-10
*/

#define _GNU_SOURCE
#include <stdio.h>
#include <sys/wait.h>
#include "common.h"
#include "pmu_event.h"
#include "cs_etm.h"
#include "cs_config.h"

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

    // configure TMC1 to be in Software FIFO mode
    cs_config_tmc1_softfifo();

    // enable PMU architectural event export
    config_pmu_enable_export();

    // initialize ETM
    config_etm_n(etms[0], 0, 1);

    // fork a child to execute the target application
    for (int i = 0; i < 1; i++)
    {
        target_pid = fork();
        if (target_pid == 0)
        {
            pin_to_core(i);
            uint64_t child_pid = (uint64_t) getpid();

            // further configure ETM. So that it will only trace the process with pid == child_pid/target_pid
            // with the program counter in the range of 0x400000 to 0x500000
            etm_set_contextid_cmp(etms[0], child_pid);
            etm_register_range(etms[0], 0x400000, 0x500000, 1);

            // When L2 cache miss happens, PMU send an input to ETM, ETM then generates an Event trace packet.
            etm_register_pmu_event(etms[0], L2D_CACHE_REFILL_T);

            // add a child process to poll RRD to read trace data
            spawn_child(poller);

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

    return 0;
}

