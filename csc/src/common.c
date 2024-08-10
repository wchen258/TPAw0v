#define _GNU_SOURCE
#include "common.h"
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

// #define CLOCK_REALTIME		((clockid_t) 1)

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