#include <stdlib.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include "trace.h"

extern void trace_loop(void);

static uint8_t * trace_buffer;
static uint32_t buffer_size;
static uint32_t buffer_pointer;

uint32_t read_data(uint8_t* buffer, uint32_t bytes, uint8_t advance_pointer)  {
    uint32_t read;

    for (read = 0; read < bytes && buffer_pointer + read < buffer_size; ++read) {
        buffer[read] = trace_buffer[buffer_pointer + read];
    }

    if (advance_pointer) {
        buffer_pointer += read;
    }

    return read;
}

uint32_t advance_pointer(uint32_t offset) {
    buffer_pointer += offset;
    if (buffer_pointer >= buffer_size)
        buffer_pointer = buffer_size;   

    return buffer_pointer;
}

uint8_t data_available() {
    return buffer_pointer < buffer_size;
}

int main(int argc, char const *argv[]) {
    int ctl_flow_fd;
    struct stat ctl_flow_stat;
    FILE * trace_file;
    char * line;
    unsigned int line_hex;
    void * ctl_ptr;
    size_t len = 0;
    ssize_t read;

    if (argc < 2) {
        fprintf(stderr, "Usage: ./ctrace [trace_input_file]\n");
        exit(EXIT_FAILURE);
    }

    trace_buffer = (uint8_t *) malloc(256 * 1024 * 1024); // 64 mb??
    buffer_size = 0;

    trace_file = fopen(argv[1], "r");
    if (trace_file == NULL) {
        fprintf(stderr, "Error opening input file %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    while ((read = getline(&line, &len, trace_file)) != -1) {
        sscanf(line, "%x", &line_hex);
        trace_buffer[buffer_size++] = line_hex & 0xFF;
        trace_buffer[buffer_size++] = (line_hex >> 8) & 0xFF;
        trace_buffer[buffer_size++] = (line_hex >> 16) & 0xFF;
        trace_buffer[buffer_size++] = (line_hex >> 24) & 0xFF;
    }

    fclose(trace_file);

    printf("Done reading file, read %d bytes (should be %d lines)\n", buffer_size, buffer_size / 4);

    trace_loop();

    return 0;
}
