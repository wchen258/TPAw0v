#ifndef ARGPARSE_H_
#define ARGPARSE_H_

#include <stdint.h>
void parse_args(int argc, char *argv[], char *app, char **app_farg, char *milestone_path, uint64_t* start_addr, uint64_t* end_addr);
void parse_args_mp(int argc, char *argv[], char *app, char **app_farg, 
                    char *milestone_path, uint64_t* start_addr, uint64_t* end_addr,
                    uint64_t* range_u, uint64_t* range_l, uint8_t* n_mp);
int argparse(int argc, char *argv[]);

#endif