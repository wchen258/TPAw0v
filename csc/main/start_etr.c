/*
    Brief: This is a simple demo to show how to use ETM to trace a target application.

    This demo should run on ZCU102/Kria board as long as the APU has linux running.
    Contrary to the original paper, this demo does not need RPU. 

    The purpose of this demo is to provide a template for researchers who want to use the CoreSight debug infrastructure.

    This demo illustrates how to use ETR to route trace data to any memory mapped buffer.

    Author: Weifan Chen
    Date: 2024-08-17
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

    // configure CoreSight to use ETR; The addr and size is the On-Chip memory (OCM) on chip.
    // You can change the addr and size to use any other
    // uint64_t buf_addr = 0x00FFE00000; // RPU 0 ATCM
    // uint32_t buf_size = 1024 * 64;
    uint64_t buf_addr = 0x00FFFC0000;  //OCM
    uint32_t buf_size = 1024 * 256;

    cs_config_etr_mp(buf_addr, buf_size);

    // clear the buffer
    clear_buffer(buf_addr, buf_size);

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

