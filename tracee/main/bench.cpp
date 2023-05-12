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

using namespace std;

extern ETM_interface *etm;

int main(int argc, char *argv[]) {

  uint64_t buf_addr = 0xb0000000;
  uint32_t buf_size = 256 * 1024 * 1024;

  char app[256];
  char *app_farg = NULL;
  char ms_path[256];
  uint64_t start_addr, end_addr;
  ms_t ms_mode;

  parse_args(argc, argv, app, &app_farg, ms_path, &ms_mode, &start_addr, &end_addr);
  clear_buffer(buf_addr, buf_size);

  // config Coresight infrascture
  cs_config_etr(buf_addr, buf_size);
  config_etm();

  // child would execl the target application, parent would wait till finish and
  // collect the results
  pid_t pid = 0;
  pid = fork();
  if (pid == 0) {

    // pin child to core 0
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(0, &set);
    sched_setaffinity(0, sizeof(cpu_set_t), &set);
    sched_yield();

    // ETM only trace child pid
    uint64_t child_pid = getpid();
    etm_set_contextid_cmp(etm, (uint64_t)child_pid);

    etm_register_start_stop_addr(etm, start_addr, end_addr);
	etm_set_stall(etm, 0b1111); // for mser, we need this to be accurate

    // enable ETM and run the application
    etm_enable(etm);
    execl(app, app, app_farg, NULL);
    fprintf(stderr, "ERROR: execl failed.\n");

  } else if (pid > 0) {
    wait(NULL);
    sleep(1);
    etm_disable(etm);
    dump_buffer(buf_addr, buf_size);
    return 0;
  } else {
    perror("Fork failed\n");
    return 1;
  }

  return 0;
}
