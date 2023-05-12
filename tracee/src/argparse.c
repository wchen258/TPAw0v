#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <argparse.h>
#include <string.h>

void parse_args(int argc, char *argv[], char *app, char **app_farg, char *milestone_path, ms_t* ms_mode, uint64_t* start_addr, uint64_t* end_addr) {
    int opt;
    while ((opt = getopt(argc, argv, ":a:m:g:b:e:")) != -1) {
        switch (opt) {
        case 'a':
            printf("First arg: %s\n", optarg);
            *app_farg = (char *) malloc(256); 
            strcpy(*app_farg, optarg);
            break;
        case 'm':
        case 'g':
            strcpy(milestone_path, optarg);
            printf("milestone path %s\n", milestone_path);
            if (opt == 'm') {
                *ms_mode = SEQUENCE;
            } else if (opt == 'g') {
                *ms_mode = GRAPH;
            }
            break;
        case 'b':
            *start_addr = strtol(optarg, NULL, 0);
            break;
        case 'e':
            *end_addr = strtol(optarg, NULL, 0);
            break;
        default:
            break;
        }
    }

    if(optind +1 == argc) {
        strcpy(app, argv[optind]);
        printf("application: %s\n", app);
    } else {
        printf("Input application required!\n");
        exit(1);
    }
}

void parse_args_mp(int argc, char *argv[], char *app, char **app_farg, 
                    char *milestone_path, ms_t* ms_mode, uint64_t* start_addr, uint64_t* end_addr,
                    uint64_t* range_u, uint64_t* range_l, uint8_t* n_mp) {
    int opt;
    while ((opt = getopt(argc, argv, ":a:m:g:b:e:u:l:n:")) != -1) {
        switch (opt) {
        case 'a':
            printf("First arg: %s\n", optarg);
            *app_farg = (char *) malloc(256); 
            strcpy(*app_farg, optarg);
            break;
        case 'm':
        case 'g':
            strcpy(milestone_path, optarg);
            printf("milestone path %s\n", milestone_path);
            if (opt == 'm') {
                *ms_mode = SEQUENCE;
            } else if (opt == 'g') {
                *ms_mode = GRAPH;
            }
            break;
        case 'b':
            *start_addr = strtol(optarg, NULL, 0);
            break;
        case 'e':
            *end_addr = strtol(optarg, NULL, 0);
            break;
        case 'u':
            *range_u = strtol(optarg, NULL, 0);
            break;
        case 'l':
            *range_l = strtol(optarg, NULL, 0);
            break;
        case 'n':
            *n_mp = strtol(optarg, NULL, 0);
            break;
        default:
            break;
        }
    }

    if(optind +1 == argc) {
        strcpy(app, argv[optind]);
        printf("application: %s\n", app);
    } else {
        printf("Input application required!\n");
        exit(1);
    }
}