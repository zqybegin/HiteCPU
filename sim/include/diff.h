#ifndef SIM_DIFF_H
#define SIM_DIFF_H

#include "memory.h"

enum {
    DIFFTEST_TO_DUT,
    DIFFTEST_TO_REF
};

typedef struct {
    word_t gpr[32];
    paddr_t pc;
} CPU_state;

extern CPU_state dut;
extern const char *regs_name[];

void difftest_init(char *ref_so_file, long img_size);
void difftest_reset();
bool difftest_step();

void show_cpu_status(CPU_state *cpu);

#endif
