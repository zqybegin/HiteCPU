#include "common.h"

void init_log(char *log_dir, char *elf_file);
void ftrace_log(word_t inst, paddr_t addr);
void close_log();
