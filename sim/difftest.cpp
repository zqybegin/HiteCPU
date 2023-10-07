#include <cstdio>
#include <dlfcn.h>
#include <memory.h>

#include "common.h"
#include "diff.h"
#include "debug.h"

CPU_state old_dut;

static bool is_skip_ref = false;

const char *regs_name[] = {"$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
                           "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
                           "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
                           "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};

using DifftestMemcpyFunc = void (*)(paddr_t, void *, size_t, bool);
using DifftestRegcpyFunc = void (*)(void *, bool);
using DifftestExecFunc = void (*)(uint64_t);
using DifftestRaiseIntrFunc = void (*)(uint64_t);
using DifftestInitFunc = void (*)(int);

DifftestMemcpyFunc ref_difftest_memcpy = NULL;
DifftestRegcpyFunc ref_difftest_regcpy = NULL;
DifftestExecFunc ref_difftest_exec = NULL;
DifftestRaiseIntrFunc ref_difftest_raise_intr = NULL;

void show_cpu_status(CPU_state *cpu) {
    printf("pc, " FMT_PADDR "\n", cpu->pc);
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 4; j++) {
            printf("%s: " FMT_WORD "\t", regs_name[i * 4 + j], cpu->gpr[i * 4 + j]);
        }
        printf("\n");
    }
}

void difftest_init(char *ref_so_file, long img_size) {
    assert(ref_so_file != NULL);

    printf(ANSI_FMT("Differential test: ON\n", ANSI_FG_BLUE));
    printf("The diff-file is in %s\n", ref_so_file);

    void *handle;
    handle = dlopen(ref_so_file, RTLD_LAZY);
    assert(handle);

    ref_difftest_memcpy = reinterpret_cast<DifftestMemcpyFunc>(dlsym(handle, "difftest_memcpy"));
    assert(ref_difftest_memcpy);

    ref_difftest_regcpy = reinterpret_cast<DifftestRegcpyFunc>(dlsym(handle, "difftest_regcpy"));
    assert(ref_difftest_regcpy);

    ref_difftest_exec = reinterpret_cast<DifftestExecFunc>(dlsym(handle, "difftest_exec"));
    assert(ref_difftest_exec);

    ref_difftest_raise_intr = reinterpret_cast<DifftestRaiseIntrFunc>(dlsym(handle, "difftest_raise_intr"));
    assert(ref_difftest_raise_intr);

    DifftestInitFunc ref_difftest_init = reinterpret_cast<DifftestInitFunc>(dlsym(handle, "difftest_init"));
    assert(ref_difftest_init);

    ref_difftest_init(0);
    ref_difftest_memcpy(CONFIG_MBASE, guest_to_host(CONFIG_MBASE), img_size, DIFFTEST_TO_REF);
}

void difftest_reset() {
    ref_difftest_regcpy(&dut, DIFFTEST_TO_REF);
}

void difftest_skip() {
    is_skip_ref = true;
}

bool difftest_checkregs(CPU_state *ref, paddr_t old_pc) {
    bool flag = true;
    if (ref->pc != dut.pc) {
        printf(ANSI_FMT("Difftest Fail: " FMT_PADDR ": ", ANSI_FG_BLUE) "$PC " FMT_PADDR "(dut) => " FMT_PADDR "(ref)\n", old_pc, dut.pc, ref->pc);
        flag = false;
    }
    for (size_t i = 0; i < 32; i++) {
        if (ref->gpr[i] != dut.gpr[i]) {
            printf(ANSI_FMT("Difftest Fail " FMT_PADDR ": ", ANSI_FG_BLUE) "%s " FMT_PADDR "(dut) => " FMT_PADDR "(ref)\n", old_pc, regs_name[i], dut.gpr[i], ref->gpr[i]);
            flag = false;
        }
    }
    return flag;
}

bool difftest_step() {
    CPU_state ref;

    if (is_skip_ref == true) {
        ref_difftest_regcpy(&dut, DIFFTEST_TO_REF);
        is_skip_ref = false;
        return true;
    }

    // ----- ref simulate model exec -----
    ref_difftest_exec(1);
    ref_difftest_regcpy(&ref, DIFFTEST_TO_DUT);
    // show_cpu_status(&dut);

    // ----- compare dut with ref -----
    bool flag = difftest_checkregs(&ref, old_dut.pc);

    if (flag == false) {
        printf(ANSI_FMT("Before inst exec: ", ANSI_FG_BLUE));
        show_cpu_status(&old_dut);
        printf(ANSI_FMT("After  inst exec: ", ANSI_FG_BLUE));
        show_cpu_status(&dut);
    }

    // ----- record cpu status of the previous cycle -----
    old_dut.pc = dut.pc;
    for (size_t i = 0; i < 32; i++) {
        old_dut.gpr[i] = dut.gpr[i];
    }

    return flag;
}
