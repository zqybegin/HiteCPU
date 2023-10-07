#ifndef SIM_DIFF_H
#define SIM_DIFF_H

#include "common.h"

enum {
    DIFFTEST_TO_DUT,
    DIFFTEST_TO_REF
};

void difftest_init(char *ref_so_file, long img_size);
void difftest_reset();
bool difftest_step();
void difftest_skip();

#endif
