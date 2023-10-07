#ifndef SIM_MAP_H
#define SIM_MAP_H

#include "common.h"

// ---------- IO Device Structure -----------
typedef void (*io_callback_t)(uint32_t, int, bool);

typedef struct {
    const char *name;
    paddr_t low;
    paddr_t high;
    void *space;
    io_callback_t callback;
} IOMap;

// ----------- mmio IO Read/Write -----------
word_t mmio_read(paddr_t addr, int len);
void mmio_write(paddr_t addr, int len, word_t data);

// ----------- mmio IO Initial -----------
void init_map();
uint8_t *new_space(int size);
void add_mmio_map(const char *name, paddr_t addr, void *space,
                  uint32_t len, io_callback_t callback);


#endif
