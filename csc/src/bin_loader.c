#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>

/*  Write the binary file [bin_name] to the target address.
    Return the first uint32_t in the binary file (the first milestone in graph) 
    --------------------
    Return:
        The first uint32_t in the binary file.
*/
uint32_t wrmem(char* bin_name, unsigned long addr) {
    FILE *file;
    char *buffer;
    long file_size;

    // Open the binary file
    file = fopen(bin_name, "rb");
    if (!file) {
        perror("Failed to open file");
        return 1;
    }

    // get the first uint32_t in the binary file
    uint32_t first_milestone;
    fread(&first_milestone, sizeof(uint32_t), 1, file);
    rewind(file);

    // Get the file size
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    rewind(file);

    // Allocate memory to hold the file's content
    buffer = (char *) malloc(file_size);
    if (!buffer) {
        perror("Memory allocation failed");
        fclose(file);
        return 1;
    }

    // Read the file into the buffer
    fread(buffer, 1, file_size, file);
    fclose(file);

    // Open the /dev/mem file
    int mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (mem_fd == -1) {
        perror("Failed to open /dev/mem");
        free(buffer);
        return 1;
    }

    // Parse the target physical address
    unsigned long physical_address = addr;

    // Calculate page boundaries and offsets
    size_t pagesize = sysconf(_SC_PAGESIZE);
    unsigned long page_base = physical_address & ~(pagesize-1);
    unsigned long page_offset = physical_address - page_base;

    // Map the physical memory address
    char *mapped_base = (char* ) mmap(0, file_size + page_offset, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, page_base);
    if (mapped_base == MAP_FAILED) {
        perror("Failed to map memory");
        close(mem_fd);
        free(buffer);
        return 1;
    }

    // Adjust the pointer to the desired address
    char *mapped_ptr = mapped_base + page_offset;

    // Write the data
    for (long i = 0; i < file_size; i++) {
        mapped_ptr[i] = buffer[i];
    }

    // Clean up
    munmap(mapped_base, file_size + page_offset);
    close(mem_fd);
    free(buffer);

    return first_milestone;
}