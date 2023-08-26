#include "VToplevel.h"
#include "verilated.h"
#include "verilated_vcd_c.h"

#include "svdpi.h"
#include "VToplevel__Dpi.h"

#include "common.h"

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
        paddr_t mem_req_addr = dut->io_mem_req_bits_addr;

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
