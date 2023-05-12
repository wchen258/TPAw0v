#include "buffer.h"
#include "cs_etm.h"
#include "cs_config.h"
#include "cs_soc.h"
#include "pmu_event.h"
#include "zcu_cs.h"
#include <bits/stdc++.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <sched.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <argparse.h>
#include <arm_perf_v8.h>

using namespace std;

extern ETM_interface *etms[4];
extern TMC_interface *tmc3;

int main(int argc, char *argv[])
{

  // set up ETR buffer. R5 refer to RPU's TCM
#ifdef R5
  uint64_t buf_addr = R5_0_ATCM + 0x8000;
  uint32_t buf_size = 8 * 1024 * 4;
#else
  uint64_t buf_addr = 0xb0000000;
  uint32_t buf_size = 256 * 1024 * 1024;
  clear_buffer(buf_addr, buf_size);
#endif

  char app[256];
  char *app_farg = NULL;
  char milestone_path[256];
  uint64_t start_addr=0;
  uint64_t end_addr=0;
  ms_t ms_mode;
  uint32_t *ms_ptr;
  uint32_t ms_size;
  cpu_set_t set;
  uint64_t range_u = 0;
  uint64_t range_l = 0;
  uint8_t n_mp = 0;

  parse_args_mp(argc, argv, app, &app_farg, milestone_path, &ms_mode, &start_addr, &end_addr,
            &range_u, &range_l, &n_mp);

  // pin the master thred to core3, the master core will not execute target application 
  printf("pin master to core3\n");
  CPU_ZERO(&set);
  CPU_SET(3, &set);
  sched_setaffinity(0, sizeof(cpu_set_t), &set);
  sched_yield();

  // config Coresight infrascture
  cs_config_etr_mp(buf_addr, buf_size);
  config_etm_n(etms[0],0,1);
  config_etm_n(etms[1],0,2);
  config_etm_n(etms[2],0,3);
  config_etm_n(etms[3],0,4);

  // fork three children, each execute a target application
  // each child is pinned to different cores
  pid_t* pids = (pid_t*) malloc(sizeof(pid_t) * n_mp);
  int i;

  for (i = 0; i < n_mp; ++i) {
    if ((pids[i] = fork()) < 0) {
      perror("fork failed");
      abort();
    } else if (pids[i] == 0) {
      CPU_ZERO(&set);
      CPU_SET(i, &set);
      sched_setaffinity(0, sizeof(cpu_set_t), &set);
      sched_yield();

      uint64_t child_pid = getpid();
      etm_set_contextid_cmp(etms[i], (uint64_t)child_pid);
      etm_register_range(etms[i], range_u, range_l, 1);
      etm_enable(etms[i]);
      execl(app, app, app_farg, NULL);
      fprintf(stderr, "ERROR: execl failed.\n");
      exit(0);
    }
  }

  /* Wait for children to exit. */
  int status;
  pid_t pid;
  int temp_n = n_mp;
  while (temp_n > 0) {
    pid = wait(&status);
    printf("Child with PID %ld exited with status 0x%x.\n", (long)pid, status);
    --temp_n;  // TODO(pts): Remove pid from the pids array.
  }
  for(i = 0; i < n_mp; ++i) {
    etm_disable(etms[i]);
  }
  tmc_man_flush(tmc3);
  sleep(1); // wait TMC3 (aka ETR) drains the buffer

  dump_buffer(buf_addr, buf_size);
#ifdef R5
  system("sed -i 's/0xDEADBEEF/0x00000000/g' ../output/trace_1.out");
#endif
  return 0;
}

