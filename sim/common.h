#ifndef SIM_COMMON_H
#define SIM_COMMON_H

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <inttypes.h>

// ----------- Memory -----------
#define MEM_SIZE 100000000
#define START_ENTRY 0x80000000

typedef uint32_t paddr_t;
typedef uint32_t word_t;

extern uint8_t pmem[MEM_SIZE];

uint8_t *guest_to_host(paddr_t paddr);
word_t mem_read(paddr_t addr, int len);
void mem_write(paddr_t addr, int len, word_t data);
long mem_init(char *img_file);

// ------- Beautiful Printf -----------
#define ANSI_FG_BLACK   "\33[1;30m"
#define ANSI_FG_RED     "\33[1;31m"
#define ANSI_FG_GREEN   "\33[1;32m"
#define ANSI_FG_YELLOW  "\33[1;33m"
#define ANSI_FG_BLUE    "\33[1;34m"
#define ANSI_FG_MAGENTA "\33[1;35m"
#define ANSI_FG_CYAN    "\33[1;36m"
#define ANSI_FG_WHITE   "\33[1;37m"
#define ANSI_BG_BLACK   "\33[1;40m"
#define ANSI_BG_RED     "\33[1;41m"
#define ANSI_BG_GREEN   "\33[1;42m"
#define ANSI_BG_YELLOW  "\33[1;43m"
#define ANSI_BG_BLUE    "\33[1;44m"
#define ANSI_BG_MAGENTA "\33[1;35m"
#define ANSI_BG_CYAN    "\33[1;46m"
#define ANSI_BG_WHITE   "\33[1;47m"
#define ANSI_NONE       "\33[0m"

#define ANSI_FMT(str, fmt) fmt str ANSI_NONE

#define FMT_WORD "0x%08" PRIx32

// ----------- Parse Argument -----------
int parse_args(int argc, char *argv[]);

// ----------- Difftest -----------
enum { DIFFTEST_TO_DUT, DIFFTEST_TO_REF };

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
