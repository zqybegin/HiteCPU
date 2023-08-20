#include "VToplevel.h"
#include "verilated.h"
#include "verilated_vcd_c.h"

#include "svdpi.h"
#include "VToplevel__Dpi.h"

#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
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

void mem_init(){
    memcpy(guest_to_host(START_ENTRY), img, sizeof(img));
}

// ----------- Verilate -----------

// DPI-C access ebreak inst
extern void ebreak(int *halt);

int main() {
    // Initial of cpu memory
    mem_init();

    // Initial of VerilatedContext
    const std::unique_ptr<VerilatedContext> contextp{new VerilatedContext};
    contextp->traceEverOn(true);

    // Initial of top module
    const std::unique_ptr<VToplevel> dut{new VToplevel{contextp.get(), "TOP"}};

    // Set vcd tracer
    VerilatedVcdC *tfp = new VerilatedVcdC;
    dut->trace(tfp, 0);
    tfp->open("dump.vcd");

    // DPI-C set scope
    const svScope scope = svGetScopeFromName("TOP.Toplevel.core.halt");
    assert(scope);  // Check for nullptr if scope not found
    svSetScope(scope);

    int halt = 0;
    while (halt != 1 && contextp->time() <= 50) {
        // ---- memory signal return ----
        // make memory like Bram, it access data by sequential logic
        // PS: if you use Bram, PC+4 will be used as the input of register PC and memory request addr at the same time,
        //     so that after the next rising edge, the address stored in PC corresponds to the data returned by the memory.
        // PS: the reset value of PC should be 0x7fff_fffc
        if (dut->clock == 0 && dut->io_mem_req_valid) {
            dut->io_mem_resp_data = mem_read(dut->io_mem_req_bits_addr, 4);
            printf("0x%08x, 0x%08x\n", dut->io_mem_req_bits_addr, mem_read(dut->io_mem_req_bits_addr, 4));
        }

        // ---- sequential signal eval ----
        dut->clock ^= 1;
        dut->eval();

        // ---- combinatorial signal eval ----
        // dut reset
        dut->reset = 0;
        if (contextp->time() < 4) dut->reset = 1;
        dut->eval();

        // make memory like Dram, it access data by combinatorial logic
        // PS: if you use Dram, PC ,not PC+4, will be used as memory request addr simply.
        //     Because the value will be return immediately.
        // PS:The place where the reset disappears will determine the initial value of the PC,
        //    because the data will be returned directly by combining logic.
        //    * if reset disappears on the falling edge, then the initial value needs to be 0x7fff_fffc.
        //    * If reset disappears on the rising edge, then the initial value.
        //    Of course, you can also set 7fffffffc as the initial value and execute an empty instruction. needs to be 0x8000_0000.
        // if (dut->io_mem_req_valid) {
        //     dut->io_mem_resp_data = mem_read(dut->io_mem_req_bits_addr, 4);
        //     printf("0x%08x, 0x%08x\n", dut->io_mem_req_bits_addr, mem_read(dut->io_mem_req_bits_addr, 4));
        // }
        // dut->eval();

        // ---- trace signal to waveform ----
        tfp->dump(contextp->time());
        contextp->timeInc(1);

        // ---- halt function to exit ----
        ebreak(&halt);
    }
    dut->final();
    tfp->close();
}
