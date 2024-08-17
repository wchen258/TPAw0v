#include <stdio.h>
#include <sys/mman.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

uint32_t *get_buf_ptr(uint64_t buf_addr, uint32_t buf_size)
{
    void* ptr = NULL;
    int fd = open("/dev/mem", O_RDWR);
    if (fd < 0) {
        perror("Cannot open /dev/mem\n");
        exit(1);
    }
    ptr = mmap(NULL, buf_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf_addr);
    if (ptr == MAP_FAILED)
		fprintf(stderr,"mmap to buffer failed!\n");
	close(fd);
    return (uint32_t *) ptr;
}

/*
    When formatter is enabled. 0xffffffff is not possible. 
    Thus it can be used as a valid trace ending marker
*/
void clear_buffer(uint64_t buf_addr, uint32_t buf_size)
{
    printf("Populate Buffer with 0xffffffff\n");
    uint32_t *ptr = get_buf_ptr(buf_addr, buf_size);
    volatile uint32_t *buf = ptr;
    for(uint32_t i=0; i<buf_size/4; i++) {
	    *buf++ = 0xffffffff;
    }
    munmap(ptr, buf_size);
}

void dump_buffer(uint64_t buf_addr, uint32_t buf_size)
{
    printf("Dumping trace to trace.{out.dat}\n");
    uint32_t *ptr = get_buf_ptr(buf_addr, buf_size);
    volatile uint32_t *buf = ptr;
    FILE *fp2 = fopen("trace.out", "w");
    FILE *fp3 = fopen("trace.dat", "w");
    if(fp2 == NULL || fp3 == NULL) {
	    printf("file can't be opened\n");
	    exit(1);
    }

    buf = ptr;
    for(uint32_t i=0; i<buf_size/4; i++) {
        if ((uint32_t)*buf != 0xffffffff) {
            fprintf(fp2, "0x%08X\n", *buf);
            fwrite((void *)buf, sizeof(uint32_t), 1, fp3);
	        buf++;
        } else {
            break;
        }
    }
    fclose(fp2);
    fclose(fp3);
    munmap(ptr, buf_size);
}
