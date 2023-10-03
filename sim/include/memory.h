#ifndef SIM_MEMORY_H
#define SIM_MEMORY_H

#include "debug.h"

#include <stdint.h>

#define CONFIG_MBASE 0x80000000
#define CONFIG_MSIZE 0x8000000

#define PMEM_LEFT  ((paddr_t)CONFIG_MBASE)
#define PMEM_RIGHT ((paddr_t)CONFIG_MBASE + CONFIG_MSIZE - 1)

typedef uint32_t paddr_t;
typedef uint32_t word_t;

extern uint8_t pmem[CONFIG_MSIZE];

uint8_t *guest_to_host(paddr_t paddr);
word_t mem_read(paddr_t addr, int len);
void mem_write(paddr_t addr, int len, word_t data);
long mem_init(char *img_file);

static inline bool in_pmem(paddr_t addr) {
    return addr - CONFIG_MBASE < CONFIG_MSIZE;
}

static inline word_t host_read(void *addr, int len) {
    switch (len) {
        case 0: return *(uint8_t *)addr;
        case 1: return *(uint16_t *)addr;
        case 2: return *(uint32_t *)addr;
        default: Assert(0, "host_read len is wrong!");
    }
}

static inline void host_write(void *addr, int len, word_t data) {
    switch (len) {
        case 0: *(uint8_t *)addr = data; return;
        case 1: *(uint16_t *)addr = data; return;
        case 2: *(uint32_t *)addr = data; return;
        default: Assert(0, "host_write len is wrong!");
    }
}

#endif
