#include <cstdio>
#include <dlfcn.h>

#include "common.h"

CPU_state dut;
CPU_state old_dut;

const char *regs_name[] = {"$0", "ra", "sp",  "gp",  "tp", "t0", "t1", "t2",
                           "s0", "s1", "a0",  "a1",  "a2", "a3", "a4", "a5",
                           "a6", "a7", "s2",  "s3",  "s4", "s5", "s6", "s7",
                           "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};

using DifftestMemcpyFunc = void (*)(paddr_t, void*, size_t, bool);
using DifftestRegcpyFunc = void (*)(void*, bool);
using DifftestExecFunc = void (*)(uint64_t);
using DifftestRaiseIntrFunc = void (*)(uint64_t);
using DifftestInitFunc = void (*)(int);

DifftestMemcpyFunc ref_difftest_memcpy = NULL;
DifftestRegcpyFunc ref_difftest_regcpy = NULL;
DifftestExecFunc ref_difftest_exec = NULL;
DifftestRaiseIntrFunc ref_difftest_raise_intr = NULL;

void show_cpu_status(CPU_state *cpu) {
    printf("\n");
    printf("pc, " FMT_WORD "\n", cpu->pc);
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 4; j++) {
            printf("%s: " FMT_WORD "\t", regs_name[i * 4 + j], cpu->gpr[i]);
        }
        printf("\n");
    }
}

void difftest_init(char *ref_so_file, long img_size) {
    assert(ref_so_file != NULL);

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

    printf("Differential testing: %s\n", ANSI_FMT("ON", ANSI_FG_GREEN));

    ref_difftest_init(0);
    ref_difftest_memcpy(START_ENTRY, guest_to_host(START_ENTRY), img_size, DIFFTEST_TO_REF);
}

void difftest_reset() {
    ref_difftest_regcpy(&dut, DIFFTEST_TO_REF);
}

bool difftest_checkregs(CPU_state *ref) {
    bool flag = true;
    if (ref->pc != dut.pc) {
        printf("Difftest fail: $PC " FMT_WORD "(dut) => " FMT_WORD "(ref)\n", dut.pc, ref->pc);
        flag = false;
    }
    for (size_t i = 0; i < 32; i++) {
        if (ref->gpr[i] != dut.gpr[i]) {
            printf("Difftest fail: %s " FMT_WORD "(dut) => " FMT_WORD "(ref)\n", regs_name[i], dut.gpr[i], ref->gpr[i]);
            flag = false;
        }
    }
    return flag;
}

bool difftest_step() {
    CPU_state ref;

    ref_difftest_exec(1);
    ref_difftest_regcpy(&ref, DIFFTEST_TO_DUT);

    show_cpu_status(&dut);

    bool flag = difftest_checkregs(&ref);
    return flag;
}


