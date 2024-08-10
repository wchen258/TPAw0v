#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <argparse.h>
#include <string.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

// void execute_command(char *args[]) {
//     pid_t pid = fork();
//     if (pid == 0) {
//         // Child process
//         if (execvp(args[0], args) == -1) {
//             perror("execvp");
//             exit(EXIT_FAILURE);
//         }
//     } else if (pid < 0) {
//         // Fork failed
//         perror("fork");
//     } else {
//         // Parent process
//         int status;
//         waitpid(pid, &status, 0);
//     }
// }


int argparse(int argc, char *argv[])
{
    int verbose = 0;
    char *inputTmgPath = NULL;

    int opt;

    while ((opt = getopt(argc, argv, "vi:")) != -1) {
        switch (opt) {
            case 'v':
                verbose = 1;
                break;
            case 'i':
                inputTmgPath = optarg;
                break;
            default:
                fprintf(stderr, "Usage: %s [-v] [-i path] -- <command> [args...]\n", argv[0]);
                return 1;
        }
    }

    if (verbose) {
        printf("Verbose mode enabled\n");
    }

    if (inputTmgPath != NULL) {
        printf("Input TMG path: %s\n", inputTmgPath);
    }

    // Process commands after '--'
    int cmdStartIndex = optind;

    while (1)
    {
        int start_index = cmdStartIndex;
        int end_index = cmdStartIndex;
        while(cmdStartIndex < argc && strcmp(argv[cmdStartIndex], "--") != 0 ) {
            cmdStartIndex++;
            end_index++;
        }

        // treate argv[start_index:end_index] as a command
        int exec_argc = end_index - start_index + 1; // +1 for NULL
        char** exec_argv = malloc(exec_argc * sizeof(char*));

        for (int i = 0; i < exec_argc - 1; i++) {
            exec_argv[i] = argv[start_index + i];
        }
        exec_argv[exec_argc - 1] = NULL; // Null-terminate the array

        // fork a child to execute the command
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            if (execvp(exec_argv[0], exec_argv) == -1) {
                perror("execvp");
                free(exec_argv);
                return 1;
            }
        } else if (pid < 0) {
            // Fork failed
            perror("fork");
            free(exec_argv);
            return 1;
        }

        // Parent process
        if(cmdStartIndex == argc) {
            // No more commands
            break;
        } else {
            // There are more commands
            cmdStartIndex++;
        }
    }

    // wait for all children to finish
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, 0)) != -1) {
        printf("Child with PID %ld exited with status 0x%x.\n", (long)pid, status);
    }

    // Check if the reason for exiting the loop was that there are no more child processes
    if (errno == ECHILD) {
        printf("No more child processes.\n");
    } else {
        perror("waitpid");
    }

    return 0;

}

void parse_args(int argc, char *argv[], char *app, char **app_farg, char *milestone_path, uint64_t *start_addr, uint64_t *end_addr)
{
    int opt;
    while ((opt = getopt(argc, argv, ":a:m:g:b:e:")) != -1)
    {
        switch (opt)
        {
        case 'a':
            printf("First arg: %s\n", optarg);
            *app_farg = (char *)malloc(256);
            strcpy(*app_farg, optarg);
            break;
        case 'm':
        case 'g':
            strcpy(milestone_path, optarg);
            printf("milestone path %s\n", milestone_path);
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

    if (optind + 1 == argc)
    {
        strcpy(app, argv[optind]);
        printf("application: %s\n", app);
    }
    else
    {
        printf("Input application required!\n");
        exit(1);
    }
}

void parse_args_mp(int argc, char *argv[], char *app, char **app_farg,
                   char *milestone_path, uint64_t *start_addr, uint64_t *end_addr,
                   uint64_t *range_u, uint64_t *range_l, uint8_t *n_mp)
{
    int opt;
    while ((opt = getopt(argc, argv, ":a:m:g:b:e:u:l:n:")) != -1)
    {
        switch (opt)
        {
        case 'a':
            printf("First arg: %s\n", optarg);
            *app_farg = (char *)malloc(256);
            strcpy(*app_farg, optarg);
            break;
        case 'm':
        case 'g':
            strcpy(milestone_path, optarg);
            printf("milestone path %s\n", milestone_path);
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

    if (optind + 1 == argc)
    {
        strcpy(app, argv[optind]);
        printf("application: %s\n", app);
    }
    else
    {
        printf("Input application required!\n");
        exit(1);
    }
}