/*
    Brief: This is a simple demo to show how to use ETM to trace a target application.

    This demo should run on ZCU102/Kria board as long as the APU has linux running.
    Contrary to the original paper, this demo does not need RPU. 

    The purpose of this demo is to provide a template for researchers who want to use the CoreSight debug infrastructure.

    Author: Weifan Chen
    Date: 2024-08-10
*/


#define _GNU_SOURCE
#include "buffer.h"
#include "cs_etm.h"
#include "cs_config.h"
#include "cs_soc.h"
#include <fcntl.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include "bin_loader.h"
#include "common.h"
#include <sys/sysinfo.h>
#include <time.h>

extern ETM_interface *etms[4];
extern TMC_interface *tmc1;

/*
    Poller waits until ETM is enabled.
    Then it polls TMC1 (aka ETF1) to read trace data.
    When ETM is disabled again, poller prints the trace data.
*/
void poller()
{
    pin_to_core(1);
    uint32_t soft_fifo_storage[256];
    uint32_t storage_ptr = 0;
    uint32_t flush_ct = 0;

    for (int i = 0; i < 256; i++)
    {
        soft_fifo_storage[i] = 0xbeefcafe;
    }

    while (etms[0]->prog_ctrl == 0);
    while (etms[0]->prog_ctrl == 1)
    {
        // When ETF is in Software FIFO mode, poll RRD register return new data or 0xffffffff if no new data
        uint32_t tmp = tmc1->ram_read_data;
        if (tmp == 0xffffffff)
        {
            // If there is no new data to read, trigger a flush to force output buffered data. But it will trash the bus with formatter padding (i.e. bunch of zeros)
            // tmc1->formatter_flush_ctrl = 0x43; 
            flush_ct++;
        }
        else
        {
            soft_fifo_storage[storage_ptr++] = tmp;
            if (storage_ptr == 256)
            {
                while (etms[0]->prog_ctrl == 1);
            }
        }
    }
    printf("Trace session ended. Poller print trace data:\n");
    for (uint32_t i = 0; i < storage_ptr; i++)
    {
        printf("%08x\n", soft_fifo_storage[i]);
    }
    printf("\nmeta data\n");
    printf("null read count: %d\n\n", flush_ct);
}

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

    // initialize ETM
    config_etm_n(etms[0], 0, 1);

    // fork a child to execute the target application. In this case, it is /bin/ls
    for (int i = 0; i < 1; i++)
    {
        target_pid = fork();
        if (target_pid == 0)
        {
            pin_to_core(i);
            uint64_t child_pid = getpid();

            // further configure ETM. So that it will only trace the process with pid == child_pid/target_pid
            // with the program counter in the range of 0x400000 to 0x500000
            etm_set_contextid_cmp(etms[0], (uint64_t)child_pid);
            etm_register_range(etms[0], 0x400000, 0x500000, 1);

            // add a child process to poll RRD to read trace data
            pid_t pid2 = fork();
            if (pid2 == 0)
            {
                poller();
                exit(0);
            }
            else if (pid2 < 0)
            {
                perror("fork");
                exit(1);
            }

            // Enable ETM, start trace session
            etm_enable(etms[0]);

            // execute target application
            execl("/bin/ls", "ls", NULL);
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

