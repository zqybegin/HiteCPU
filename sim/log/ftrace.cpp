#include "common.h"
#include "log/ftrace.h"
#include "debug.h"

uint64_t depth = 0;

#define BITMASK(bits) ((1ull << (bits)) - 1)
#define BITS(x, hi, lo) (((x) >> (lo)) & BITMASK((hi) - (lo) + 1)) // similar to x[hi:lo] in verilog
// https://gcc.gnu.org/onlinedocs/gcc/Statement-Exprs.html
#define SEXT(x, len) ({ struct { int64_t n : len; } __x = { .n = (int64_t)x }; (uint64_t)__x.n; })

#define LINK(A) ((A) == 1 || (A) == 5)
#define PRINT_DEPTH(A) for(uint64_t i = 0; i < A; i++)\
            fprintf(ftrace_fp, "  ")

#define ZEROMASK(bits) ((0ull - 1) << (bits))
#define ZERO(x, bits) (x & ZEROMASK(bits)) //simalr to x[bits-1:0] = 0

enum {
    JAL,
    JALR,
    NONE
};

int inst_type(word_t inst) {
    if (BITS(inst, 6, 0) == 0x6f) {
        return JAL;
    } else if (BITS(inst,6,0) == 0x67 && BITS(inst,14,12)== 0x0) {
        return JALR;
    } else {
        return NONE;
    }
}


void ftrace_log(word_t inst, paddr_t addr){
    int type = inst_type(inst);
    if (type == NONE) return;

    int rd = BITS(inst, 11, 7);
    int rs1 = BITS(inst, 19, 15);

    fprintf(ftrace_fp, FMT_WORD " ", addr);
    if (type == JAL && LINK(rd)) {
        word_t imm = (SEXT(BITS(inst, 31, 31), 1) << 20) | (BITS(inst, 30, 21) << 1) | (BITS(inst, 20, 20) << 11) | (BITS(inst, 19, 12) << 12);
        paddr_t jump_addr = addr + imm;
        char *func_name = FuncList_search(func_list, jump_addr);
        PRINT_DEPTH(depth);
        fprintf(ftrace_fp, "call [%s@" FMT_WORD "]\n", func_name, addr);
        depth++;
    }

    if (type == JALR && (LINK(rd) || LINK(rs1)) ) {
        word_t imm = SEXT(BITS(inst, 31, 20), 12);
        paddr_t jump_addr = ZERO((dut.gpr[rs1] + imm), 1);
        if ( !LINK(rd) && LINK(rs1) ) {
            // search func_name according jump addr
            char *func_name = FuncList_search(func_list, addr);
            PRINT_DEPTH(depth - 1);
            fprintf(ftrace_fp, "ret  [%s@" FMT_WORD "]\n", func_name, addr);
            depth--;
        } else if ( LINK(rd) && !LINK(rs1) ) {
            // search func_name according jump addr
            char *func_name = FuncList_search(func_list, addr);
            PRINT_DEPTH(depth);
            fprintf(ftrace_fp, "call [%s@" FMT_WORD "]\n", func_name, addr);
            depth++;
        } else if (rd != rs1) {
            // search func_name according jump addr
            char *func_name = FuncList_search(func_list, addr);
            // LINK(rd) && LINK(rs1) && rd != rs1: push and pop
            PRINT_DEPTH(depth);
            fprintf(ftrace_fp, "ret and call [%s@" FMT_WORD "]\n", func_name, addr);
        } else {
            // search func_name according jump addr
            char *func_name = FuncList_search(func_list, addr);
            // LINK(rd) && LINK(rs1) && rd == rs1: push and pop
            PRINT_DEPTH(depth);
            fprintf(ftrace_fp, "call [%s@" FMT_WORD "]\n", func_name, addr);
            depth++;
        }
    }
}
