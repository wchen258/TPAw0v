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

extern ETM_interface *etm;

tuple<uint32_t *, int> read_ms(string fname)
{
  vector<uint32_t> v;
  ifstream msfile(fname);
  string str;
  if (msfile.is_open())
  {
    while (getline(msfile, str, ','))
    {
      cout << str << endl;
      v.push_back(stoul(str, nullptr, 16));
    }
  }
  else
  {
    cout << "Milestone file faied to open" << endl;
  }
  size_t num_ms = v.size();
  uint32_t *ms = (uint32_t *)malloc(sizeof(uint32_t) * v.size());
  for (size_t i = 0; i < num_ms; i++)
  {
    ms[i] = v[i];
  }
  return make_pair(ms, num_ms);
}

int get_file_size(char *fname)
{
  FILE *fp = fopen(fname, "rb");
  int sz;
  fseek(fp, 0L, SEEK_END);
  sz = ftell(fp);
  return sz;
}

tuple<uint32_t *, int> read_msg(char *fname)
{
  int size = get_file_size(fname);
  uint32_t *buf = (uint32_t *)malloc(size);
  FILE *fp = fopen(fname, "rb");
  fread(buf, sizeof(uint32_t), size / sizeof(uint32_t), fp);
  return make_pair(buf, size / sizeof(uint32_t));
}

void write_ms_time(uint32_t *ms, uint32_t *ms_time, int ms_size)
{
  ofstream msfile("ms_timing.txt");
  if (!msfile.is_open())
  {
    cout << "ERROR: cannot open ms_timing.txt, timing info does not write to "
            "the file"
         << endl;
    return;
  }
  for (int i = 0; i < ms_size; i++)
  {
    msfile << "0x" << hex << ms[i];
    if (i == ms_size - 1)
    {
      msfile << endl;
    }
    else
    {
      msfile << ",";
    }
  }
  for (int i = 0; i < ms_size; i++)
  {
    msfile << dec << ms_time[i];
    if (i == ms_size - 1)
    {
      msfile << endl;
    }
    else
    {
      msfile << ",";
    }
  }
  cout << "Timing info write to ms_timing.txt" << endl;
}

void pmu_event_setup(unsigned int e0, unsigned int e1, unsigned int e2, unsigned int e3, unsigned int e4, unsigned int e5) {
  arm_perf_disable_counter(0x3f);
  arm_perf_set_ctrl(ARM_PERF_PMCR_P);
  arm_perf_type0(e0);
  arm_perf_type1(e1);
  arm_perf_type2(e2);
  arm_perf_type3(e3);
  arm_perf_type4(e4);
  arm_perf_type5(e5);
}

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

  parse_args(argc, argv, app, &app_farg, milestone_path, &ms_mode, &start_addr, &end_addr);

  if (ms_mode == SEQUENCE)
  {
    printf("Sequence milestone mode\n");
    tie(ms_ptr, ms_size) = read_ms(milestone_path);
  }
  else if (ms_mode == GRAPH)
  {
    printf("Graph milestone mode\n");
    tie(ms_ptr, ms_size) = read_msg(milestone_path);
  }

  // config Coresight infrascture
  cs_config_etr(buf_addr, buf_size);
  config_etm();

  // child would execl the target application, parent would wait till finish and
  // collect the results
  pid_t pid = 0;
  pid = fork();
  if (pid == 0)
  {

    // pin child to core 0
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(0, &set);
    sched_setaffinity(0, sizeof(cpu_set_t), &set);
    sched_yield();

    // ETM only trace child pid
    uint64_t child_pid = getpid();
    etm_set_contextid_cmp(etm, (uint64_t)child_pid);

    if (start_addr != 0 && end_addr != 0) {
      etm_register_start_stop_addr(etm, start_addr, end_addr);
    }

    // Write MS data to OCM so that RPU can read
    int mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
    uint32_t *ms_buff =
        (uint32_t *)mmap(0, getpagesize(), PROT_READ | PROT_WRITE, MAP_SHARED,
                         mem_fd, 0xfffc0000);

    uint32_t i;
    for (i = 0; i < ms_size; ++i)
      ms_buff[2 + i] = ms_ptr[i];
    ms_buff[1] = ms_size;
    ms_buff[0] = 0xdeadbeef;

    for (i = 0; i < 4; ++i)
    {
      if (ms_mode == SEQUENCE)
      {
        etm_register_range(etm, ms_ptr[i], ms_ptr[i], 1);
      }
      else if (ms_mode == GRAPH)
      {
        etm_register_range(etm, 0, 0, 1);
        // etm_register_single_addr_match_event(etm, milestones[i]);
      }
    }
    printf("Driver finished config, wait for Tracer init...\n");
    sleep(2);

    // set PMU event, but do not start counting. The counting should start by RPU responding to the 1st MS hit.
    //pmu_event_setup(ARM_PERF_EVENT_DC2W, ARM_PERF_EVENT_DC2R, 0,0,0,0);
    //arm_perf_enable_counter(3);
    //arm_perf_set_ctrl(ARM_PERF_PMCR_E);

    // enable ETM and run the application
    munmap((uint32_t*)0xfffc0000, getpagesize());
    etm_enable(etm);
    //int test0,test1;
    //test0 = arm_perf_counter0();
    //test1 = arm_perf_counter1();
    //printf("test counter %d %d \n", test0, test1);
    execl(app, app, app_farg, NULL);
    fprintf(stderr, "ERROR: execl failed.\n");
  }
  else if (pid > 0)
  {
    wait(NULL);
    sleep(1);
    etm_disable(etm);

    // int mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
    // uint32_t *ms_buff =
    //     (uint32_t *)mmap(0, getpagesize(), PROT_READ | PROT_WRITE, MAP_SHARED,
    //                      mem_fd, 0xfffc0000);
    // ms_buff = &ms_buff[2];

    // int i = 0;
    // while (ms_buff[i] != 0)
    // {
    //   printf("RPU: %d: %u\n\r", i, ms_buff[i]);
    //   i++;
    // }

    // write_ms_time(ms_ptr, ms_buff, ms_size);
    dump_buffer(buf_addr, buf_size);
#ifdef R5
    system("sed -i 's/0xDEADBEEF/0x00000000/g' ../output/trace_1.out");
#endif

    return 0;
  }
  else
  {
    perror("Fork failed\n");
    return 1;
  }

  return 0;
}
