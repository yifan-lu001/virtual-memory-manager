#ifndef INTERFACE_H
#define INTERFACE_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/ucontext.h>
#include <stdint.h>
#include <math.h>

// Policy type
enum policy_type
{
    MM_FIFO = 1,  // FIFO Replacement Policy
    MM_THIRD = 2, // Third Chance Replacement Policy
};

// APIs
void mm_init(enum policy_type policy, void *vm, int vm_size, int num_frames, int page_size);

void mm_logger(int virt_page, int fault_type, int evicted_page, int write_back, unsigned int phy_addr);

int get_physical_address(int page_number, int offset);
void handler_fifo(int signum, siginfo_t *info, void *ucontext);
void handler_third_chance(int signum, siginfo_t *info, void *ucontext);

#endif