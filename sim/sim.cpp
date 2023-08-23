#include "VToplevel.h"
#include "verilated.h"
#include "verilated_vcd_c.h"

#include "svdpi.h"
#include "VToplevel__Dpi.h"

#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

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
// ----------- MEM -----------
#define MEM_SIZE 100000000
#define START_ENTRY 0x80000000

typedef uint32_t addr_t;
typedef uint32_t word_t;

static uint8_t pmem[MEM_SIZE] = {};
static const uint32_t img[] = {
    0x00100113,
    0x00110113,
    0x00110113,
    0x00110113,
    0x00100073,
    // 0000000 00001 00010 000 00010 00100 11
    // 0000 0000 0001 0001 0000 0001 0001 0011
    // 0    0    1    1    0    1    1    3
};

uint8_t *guest_to_host(addr_t paddr) { return pmem + paddr - START_ENTRY; }

word_t mem_read(addr_t addr, int len) {
    switch (len) {
        case 1: return *(uint8_t  *)guest_to_host(addr);
        case 2: return *(uint16_t *)guest_to_host(addr);
        case 4: return *(uint32_t *)guest_to_host(addr);
        default: assert(0);
    }
}

void mem_write(addr_t addr, int len, word_t data) {
    switch (len) {
        case 1: *(uint8_t  *)guest_to_host(addr) = data; return;
        case 2: *(uint16_t *)guest_to_host(addr) = data; return;
        case 4: *(uint32_t *)guest_to_host(addr) = data; return;
    }
}

void mem_init(char *img_file) {
    if (img_file == NULL) {
        printf(ANSI_FMT("No image is given. Use the default build-in image.\n", ANSI_FG_YELLOW));
        memcpy(guest_to_host(START_ENTRY), img, sizeof(img));
        return;
    }

    FILE *fp = fopen(img_file, "rb");
    assert(fp != NULL);

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);

    printf("The image is %s, size = %ld\n", img_file, size);

    fseek(fp, 0, SEEK_SET);
    int ret = fread(guest_to_host(START_ENTRY), size, 1, fp);
    assert(ret == 1);

    fclose(fp);
    return;
}
// ----------- Verilate -----------

// DPI-C access ebreak inst
extern void ebreak(int *halt_valid, int *halt_value);

int main(int argc, char *argv[]) {
    assert(argc == 2 || argc == 3);

    // Initial of cpu memory
    char *img_file = NULL;
    if (argc == 3) img_file = argv[2];
    mem_init(img_file);

    // Initial of VerilatedContext
    const std::unique_ptr<VerilatedContext> contextp{new VerilatedContext};
    contextp->traceEverOn(true);

    // Initial of top module
    const std::unique_ptr<VToplevel> dut{new VToplevel{contextp.get(), "TOP"}};

    // Set vcd tracer
    VerilatedVcdC *tfp = new VerilatedVcdC;
    dut->trace(tfp, 0);
    tfp->open(argv[1]);

    // DPI-C set scope
    const svScope scope = svGetScopeFromName("TOP.Toplevel.core.halt");
    assert(scope);  // Check for nullptr if scope not found
    svSetScope(scope);

    int halt_valid = 0;
    int halt_value = -1;
    while (halt_valid != 1 && contextp->time() < 50) {
        // dut reset, because reset should be valid before all signal eval
        dut->reset = 0;
        if (contextp->time() < 3){
            dut->reset = 1;
        }

        // ---- memory signal return ----
        addr_t mem_req_addr = dut->io_mem_req_bits_addr;

        // ---- sequential signal eval ----
        dut->clock ^= 1;
        dut->eval();

        // ---- memory signal return ----
        // make memory like Bram, it access data by sequential logic
        /* PS: if you use Bram, PC+4 will be used as the input of register PC and memory request addr at the same time,
         *     so that after the next rising edge, the address stored in PC corresponds to the data returned by the memory.
         * PS: the reset value of PC should be 0x7fff_fffc
         * Important: You cannot place the mem-return-logic before dut->eval(). Otherwise, the data will be returned before
         *            the rising edge of the clock, and the rising edge of the clock will calculate the combinatorial logic
         *            behavior caused by the data return, which does not conform to our understanding of timing, and leads to
         *            extremely confusing waveforms. And our expected behavior is: after the rising edge of the clock,
         *            request data to return, and calculate its behavior before the next rising edge of the clock arrives.
         */
        if (dut->clock == 1 && dut->io_mem_req_valid) {
            dut->io_mem_resp_data = mem_read(mem_req_addr, 4);
            printf("0x%08x, 0x%08x\n", mem_req_addr, mem_read(mem_req_addr, 4));
        }
        dut->eval();

        // make memory like Dram, it access data by combinatorial logic
        /* PS: if you use Dram, PC ,not PC+4, will be used as memory request addr simply.
         *     Because the value will be return immediately.
         * PS:The place where the reset disappears will determine the initial value of the PC,
         *    because the data will be returned directly by combining logic.
         *    * if reset disappears on the falling edge, then the initial value needs to be 0x7fff_fffc.
         *    * If reset disappears on the rising edge, then the initial value.
         *    Of course, you can also set 7fffffffc as the initial value and execute an empty instruction. needs to be 0x8000_0000.
         */
        // if (dut->io_mem_req_valid) {
        //     dut->io_mem_resp_data = mem_read(dut->io_mem_req_bits_addr, 4);
        //     printf("0x%08x, 0x%08x\n", dut->io_mem_req_bits_addr, mem_read(dut->io_mem_req_bits_addr, 4));
        // }
        // dut->eval();

        // ---- combinatorial signal eval ----


        // ---- trace signal to waveform ----
        tfp->dump(contextp->time());
        contextp->timeInc(1);

        // ---- halt function to exit ----
        ebreak(&halt_valid, &halt_value);
    }

    // print hit good or fail

    printf( (halt_value == 0? ANSI_FMT("HIT GOOD TRAP\n", ANSI_FG_GREEN) : ANSI_FMT("HIT BAD TRAP\n", ANSI_FG_RED)) );

    dut->final();
    tfp->close();

    return halt_value;
}
