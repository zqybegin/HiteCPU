#include <cstdio>
#include <stdlib.h>

#include "map.h"
#include "diff.h"
#include "debug.h"
#include "memory.h"
#include "debug.h"

// ---------- IO Device Structure -----------
#define NR_MAP 16

static IOMap maps[NR_MAP] = {};
static int nr_map = 0;

static uint8_t *io_space = NULL;
static uint8_t *p_space = NULL;

#define IO_SPACE_MAX (2 * 1024 * 1024)

// ----------- mmio IO Read/Write -----------
static inline bool map_inside(IOMap *map, paddr_t addr) {
    return (addr >= map->low && addr <= map->high);
}

static inline int find_mapid_by_addr(IOMap *maps, int size, paddr_t addr) {
    for (int i = 0; i < size; i++) {
        if (map_inside(maps + i, addr)) {
            difftest_skip();
            return i;
        }
    }
    return -1;
}

static void invoke_callback(io_callback_t c, paddr_t offset, int len, bool is_write) {
    if (c != NULL) {
        c(offset, len, is_write);
    }
}

static void check_bound(IOMap *map, paddr_t addr) {
    if (map == NULL) {
        Assert(map != NULL,
               "address (" FMT_PADDR ") is out of bound at pc = " FMT_PADDR,
               addr, dut.pc);
    } else {
        Assert(addr <= map->high && addr >= map->low,
               "address (" FMT_PADDR ") is out of bound {%s} [" FMT_PADDR
               ", " FMT_PADDR "] at pc = " FMT_PADDR,
               addr, map->name, map->low, map->high, dut.pc);
    }
}

word_t mmio_read(paddr_t addr, int len) {
    int mapid = find_mapid_by_addr(maps, nr_map, addr);
    Assert(mapid != -1, "MMIO can't find IO device!");
    IOMap *map = &maps[mapid];
    Assert(len >= 0 && len <= 2, "len is %d\n", len);
    check_bound(map, addr);
    paddr_t offset = addr - map->low;
    invoke_callback(map->callback, offset, len, false); // prepare data to read
    return host_read(map->space + offset, len);
}

void mmio_write(paddr_t addr, int len, word_t data) {
    int mapid = find_mapid_by_addr(maps, nr_map, addr);
    Assert(mapid != -1, "MMIO can't find IO device!");
    IOMap *map = &maps[mapid];
    Assert(len >= 0 && len <= 2, "len is %d\n", len);
    check_bound(map, addr);
    paddr_t offset = addr - map->low;
    host_write(map->space + offset, len, data);
    invoke_callback(map->callback, offset, len, true);
}

// ----------- mmio IO Initial -----------
static void report_mmio_overlap(const char *name1, paddr_t l1, paddr_t r1,
                                const char *name2, paddr_t l2, paddr_t r2) {
    Panic("MMIO region %s@[" FMT_PADDR ", " FMT_PADDR "] is overlapped "
          "with %s@[" FMT_PADDR ", " FMT_PADDR "]",
          name1, l1, r1, name2, l2, r2);
}

void add_mmio_map(const char *name, paddr_t addr, uint8_t *space,
                  uint32_t len, io_callback_t callback) {
    assert(nr_map < NR_MAP);
    paddr_t left = addr, right = addr + len - 1;
    if (in_pmem(left) || in_pmem(right)) {
        report_mmio_overlap(name, left, right, "pmem", PMEM_LEFT, PMEM_RIGHT);
    }
    for (int i = 0; i < nr_map; i++) {
        if (left <= maps[i].high && right >= maps[i].low) {
            report_mmio_overlap(name, left, right, maps[i].name, maps[i].low,
                                maps[i].high);
        }
    }

    maps[nr_map] = (IOMap){.name = name,
                           .low = addr,
                           .high = addr + len - 1,
                           .space = space,
                           .callback = callback};
    printf("Add mmio map '%s' at [" FMT_PADDR ", " FMT_PADDR "]\n",
           maps[nr_map].name, maps[nr_map].low, maps[nr_map].high);

    nr_map++;
}

uint8_t *new_space(int size) {
    uint8_t *p = p_space;
    // page aligned;
    size = (size + (PAGE_SIZE - 1)) & ~PAGE_MASK;
    p_space += size;
    assert(p_space - io_space < IO_SPACE_MAX);
    return p;
}

void init_map() {
    io_space = (uint8_t *)malloc(IO_SPACE_MAX);
    assert(io_space);
    p_space = io_space;
}
