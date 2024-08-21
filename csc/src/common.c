#define _GNU_SOURCE
#include "common.h"
#include "cs_etm.h"
#include "cs_soc.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>
#include <sched.h>
#include <errno.h>
#include <sys/sysinfo.h>
#include <time.h>

// spawn a child process for the input function
void spawn_child(void (*func)())
{
    pid_t pid = fork();
    if (pid == 0)
    {
        func();
        exit(0);
    }
    else if (pid < 0)
    {
        perror("fork");
        exit(1);
    }
}

/*
    Poller waits until ETM is enabled.
    Then it polls TMC1 (aka ETF1) to read trace data.
    When ETM is disabled again, poller prints the trace data.
*/
extern ETM_interface *etms[4];
extern TMC_interface *tmc1;
void poller()
{
    pin_to_core(1);
    const uint32_t storage_size = 1024 * 32;
    uint32_t soft_fifo_storage[storage_size];
    uint32_t storage_ptr = 0;
    uint32_t flush_ct = 0;

    while (etms[0]->prog_ctrl == 0);
    while (etms[0]->prog_ctrl == 1 || !etm_is_idle(etms[0]) || !(tmc1->ram_write_pt == tmc1->ram_read_pt))
    {
        // When ETF is in Software FIFO mode, poll RRD register return new data or 0xffffffff if no new data
        uint32_t tmp = tmc1->ram_read_data;
        if (tmp == 0xffffffff)
        {
            // If there is no new data to read, trigger a flush to force output buffered data. But it will trash the bus with formatter padding (i.e. bunch of zeros)
            // tmc1->formatter_flush_ctrl = 0x43; 
            flush_ct++;
            if (flush_ct % 10 == 0)
            {
                // etm_print_large_counter(etms[0], 0); // it's very hacky, but you can print cnt value at runtime
            }
        }
        else
        {
            soft_fifo_storage[storage_ptr++] = tmp;
            if (storage_ptr == storage_size)
            {
                while (etms[0]->prog_ctrl == 1);
            }
        }
    }
    printf("Trace session ended. Poller print trace data:\n");

    // open a new file name trace.out and write the trace data to it.
    FILE *fp = fopen("trace.out", "w");

    // open a new file to save binary version
    FILE *fp_bin = fopen("trace.dat", "wb");

    for (uint32_t i = 0; i < storage_ptr; i++)
    {
        fprintf(fp, "0x%08x\n", soft_fifo_storage[i]);
        fwrite(&soft_fifo_storage[i], sizeof(uint32_t), 1, fp_bin);
    }

    printf("Trace snippet 0 - 30 (line) \n");
    for (uint32_t i = 0; i < storage_ptr; i++)
    {
        printf("0x%08x\n", soft_fifo_storage[i]);
        if(i == 30) break;
    }

    fclose(fp);
    fclose(fp_bin);

    printf("\nmeta data\n");
    printf("null read count: %d\n\n", flush_ct);
    printf("total read count: %d\n", storage_ptr);
    printf("Trace data is saved to trace.out/dat\n");
}

int write_mem(unsigned long physical_address, uint32_t data)
{

    int mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (mem_fd == -1) {
        perror("Failed to open /dev/mem");
        return 1;
    }

    size_t pagesize = sysconf(_SC_PAGESIZE);
    unsigned long page_base = physical_address & ~(pagesize-1);
    unsigned long page_offset = physical_address - page_base;

    // Map the physical memory address
    char *mapped_base = (char* ) mmap(0, pagesize, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, page_base);
    if (mapped_base == MAP_FAILED) {
        perror("Failed to map memory");
        close(mem_fd);
        return 1;
    }

    // Adjust the pointer to the desired address
    uint32_t *mapped_ptr = (uint32_t *) (mapped_base + page_offset);
    *mapped_ptr = data;

    close(mem_fd);
    munmap(mapped_base, pagesize);

    return 0;
}


/*  Pin the calling process to core[id] */
void pin_to_core(uint8_t id) {
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(id, &set);
    sched_setaffinity(0, sizeof(cpu_set_t), &set);
    sched_yield();
}

/* disable cpuidle for the given core
 *
 * returns 0 on success, error otherwise
 */
static int linux_disable_cpuidle_cpu(unsigned int cpu) //
{
	char s[80];
	ssize_t w;
	int err;
	int fd;

	sprintf(s, "/sys/devices/system/cpu/cpu%u/cpuidle/state1/disable", cpu);

	fd = open(s, O_RDWR);
	if (fd == -1) {
		err = errno;
		if (err == ENOENT) {
			/* on "file-not-found", cpuidle is already deactivated */
			err = 0;
		}
		return err;
	}

	w = write(fd, "1", 1);
	err = errno;

	close(fd);

	return w == 1 ? 0 : err;
}

void linux_disable_cpuidle(void)
{
	struct timespec ts;
	unsigned int num_cpus;
	unsigned int cpu;
	int err;

	err = 0;
	num_cpus = get_nprocs();
	for (cpu = 0; cpu < num_cpus; cpu++) {
		err |= linux_disable_cpuidle_cpu(cpu);
	}
	if (err == 0) {
		printf("# info: cpuidle for all %u CPUs disabled\n", num_cpus);
	} else {
		printf("warning: cannot disable cpuidle for CPUs, run as root!\n");
	}

	/* now wait 10ms for the changes to become active
	 *
	 * NOTE: if the controller crashes with a bus error like this:
	 *   Regulator on M4 core
	 *   version: 5, 4 CPUs, 128 history, 2 samples
	 *   buildid: azu@hallertau 2023-02-01 22:31:41 RELEASE VERBOSE
	 *   TSC running at 200000000 Hz
	 *   waiting for start signal from main cores: OK
	 *   Panic: fault 5 bus:
	 *   r0  00000000  r1  c5acce55  r2  28430f00  r3  00000000
	 *   r4  00001980  r5  e0001004  r6  20000c40  r7  00001980
	 *   r8  00001a68  r9  00000000  r10 00000000  r11 00000000
	 *   r12 00000000  sp  20000fe0  lr  0000174b  pc  00000e14
	 *   psr 41000200  fsr 00000400 addr e000ed38
	 * then increase the timeout. Empirically, 10ms was enough.
	 */
	ts.tv_sec = 0;
	ts.tv_nsec = 10*1000*1000;
	clock_nanosleep(CLOCK_REALTIME, 0, &ts, NULL);
}

int buff_read() {
	uint64_t buf_addr = 0xFFFC0000;
	// uint64_t buf_addr = 0xFFE20000;

    uint32_t buf_size = 1024 * 4 * 8;  // 32 KB

    int fd = open("/dev/mem", O_RDONLY);
    if (fd == -1) {
        perror("Error opening /dev/mem");
        return 1;
    }

    void *mapped_base = mmap(0, buf_size, PROT_READ, MAP_SHARED, fd, buf_addr);
    if (mapped_base == MAP_FAILED) {
        perror("Error with mmap");
        close(fd);
        return 1;
    }

    sleep(1);
    printf("Reading data from 0x%lx:\n", buf_addr);
    uint8_t *data = (uint8_t*)mapped_base; // each time per byte
    for (uint32_t i = 0; i < 64; i++) {
        if (i % 16 == 0) {
            printf("\n0x%lx: ", buf_addr + i);
        }
        printf("%02x ", data[i]);
        if ((i + 1) % 8 == 0) {
            printf(" ");
        }
    }
    printf("\n");

    munmap(mapped_base, buf_size);
    close(fd);

    return 0;
}


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