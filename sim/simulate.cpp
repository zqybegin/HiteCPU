#include "VToplevel.h"
#include "VToplevel___024root.h"

#include "verilated.h"
#include "verilated_vcd_c.h"

#include "svdpi.h"
#include "VToplevel__Dpi.h"

#include "common.h"
#include "diff.h"
#include "memory.h"
#include "debug.h"
#include "log.h"

// CPU state
CPU_state dut;

// DPI-C access ebreak inst
extern void ebreak(int *halt_valid, int *halt_value);

// Paser argument result
char *img_file, *diff_so_file, *vcd_file, *log_dir, *elf_file;
extern int parse_args(int argc, char *argv[]);
extern void init_device();
extern void init_log(char *log_dir, char *elf_file);

// Read regfile from simulator model
void get_cpu_status(const std::unique_ptr<VToplevel> &npc) {
    dut.pc = npc->rootp->Toplevel__DOT__core__DOT__pc;
    for (int i = 0; i < 32; i++) {
        dut.gpr[i] = npc->rootp->Toplevel__DOT__core__DOT__regfile__DOT__regs[i];
    }
}

int main(int argc, char *argv[]) {
    // -----------------------------------------------------------------
    //                   Initial of utils
    // -----------------------------------------------------------------
    Assert(parse_args(argc, argv) == 0, "Reading arguments Error!");
    long img_size = mem_init(img_file); // Initial of Memory
    IFDEF(CONFIG_DIFFTEST, difftest_init(diff_so_file, img_size)); // Initial of Difftest
    IFDEF(CONFIG_DEVICE, init_device()); // Initial of device
    IFDEF(CONFIG_FTRACE, init_log(log_dir, elf_file)); // Initial of log

    // -----------------------------------------------------------------
    //                   Initial of Verilator
    // -----------------------------------------------------------------
    // Initial of VerilatedContext
    const std::unique_ptr<VerilatedContext> contextp{new VerilatedContext};
    contextp->traceEverOn(true);

    // Initial of top module
    const std::unique_ptr<VToplevel> npc{
        new VToplevel{contextp.get(), "TOP"}
    };

    // Set vcd tracer
    VerilatedVcdC *tfp = new VerilatedVcdC;
    npc->trace(tfp, 0);
    tfp->open(vcd_file); // need vcd_file read by parse_args

    // DPI-C set scope
    const svScope scope = svGetScopeFromName("TOP.Toplevel.core.halt");
    assert(scope); // Check for nullptr if scope not found
    svSetScope(scope);

    // -----------------------------------------------------------------
    //                    Core of simulation
    // -----------------------------------------------------------------
    bool before_reset = false, difftest_flag = true;
    int difftest_over = 0;
    int halt_valid = 0, halt_value = -1;

    // Difftest needs to record the cycle where the error occurs, so it needs to delay the end of the simulation by 2 beats
    while (difftest_over != 1 && halt_valid != 1) {
        // npc reset, because reset should be valid before all signal eval
        npc->reset = 0;
        if (contextp->time() < 3) {
            npc->reset = 1;
        }

        // ---- inst memory addr signal record ----
        paddr_t mem_req_addr = npc->io_imem_req_bits_addr;

        // ---- sequential signal eval ----
        npc->clock ^= 1;
        npc->eval();

        // ---- inst memory data signal return ----
        /* make memory like Bram, it access data by sequential logic
         * PS: if you use Bram, PC+4 will be used as the input of register PC and memory request addr at the same time,
         *     so that after the next rising edge, the address stored in PC corresponds to the data returned by the memory.
         * PS: the reset value of PC should be 0x7fff_fffc
         * Important: You cannot place the mem-return-logic before npc->eval(). Otherwise, the data will be returned before
         *            the rising edge of the clock, and the rising edge of the clock will calculate the combinatorial logic
         *            behavior caused by the data return, which does not conform to our understanding of timing, and leads to
         *            extremely confusing waveforms. And our expected behavior is: after the rising edge of the clock,
         *            request data to return, and calculate its behavior before the next rising edge of the clock arrives.
         *
         * PS:when use Dram, the place where the reset disappears will determine the initial value of the PC,
         *    because the data will be returned directly by combining logic.
         *    * if reset disappears on the falling edge, then the initial value needs to be 0x7fff_fffc.
         *    * If reset disappears on the rising edge, then the initial value.
         *    Of course, you can also set 7fffffffc as the initial value and execute an empty instruction. needs to be 0x8000_0000.
         */
        if (npc->clock == 1 && npc->io_imem_req_valid) {
            npc->io_imem_resp_data = mem_read(mem_req_addr, 2);
            // printf( FMT_WORD ", " FMT_WORD "\n", mem_req_addr, mem_read(mem_req_addr, 4));
        }
        npc->eval();

        if (npc->clock == 0 && npc->io_dmem_req_valid) {
            if (npc->io_dmem_req_bits_wr) {
                // printf("Write " FMT_WORD ", " FMT_WORD "\n", npc->io_dmem_req_bits_addr, npc->io_dmem_req_bits_size);
                // printf(FMT_WORD "\n", npc->io_dmem_req_bits_addr - CONFIG_MBASE < CONFIG_MSIZE);
                mem_write(npc->io_dmem_req_bits_addr, npc->io_dmem_req_bits_size, npc->io_dmem_req_bits_data);
            } else {
                // printf("READ  " FMT_WORD ", " FMT_WORD "\n", npc->io_dmem_req_bits_addr, npc->io_dmem_req_bits_size);
                // printf(FMT_WORD "\n", npc->io_dmem_req_bits_addr - CONFIG_MBASE < CONFIG_MSIZE);
                npc->io_dmem_resp_data = mem_read(npc->io_dmem_req_bits_addr, npc->io_dmem_req_bits_size);
            }
        }
        npc->eval();

        // ---- trace signal to waveform ----
        tfp->dump(contextp->time());
        contextp->timeInc(1);


#ifdef CONFIG_DIFFTEST
        // ---- difftest ----
        if (npc->clock == 1 && npc->reset == 0) {
            // get cpu state from verilator model
            get_cpu_status(npc);
            if (before_reset) {
                // reset ref in the first cycle after reset signal disappears
                difftest_reset();
            } else {
                // difftest core to compare
                if (difftest_flag == true) {
                    difftest_flag = difftest_step();
                } else {
                    difftest_over++;
                }
            }
        }
        // record the reset signal of the previous cycle
        if (npc->clock == 1)
            before_reset = (npc->reset == 1);
#endif

#ifdef CONFIG_FTRACE
        if (npc->clock == 1) {
            ftrace_log(npc->io_imem_resp_data, mem_req_addr);
        }
#endif

        // ---- halt function to exit ----
        ebreak(&halt_valid, &halt_value);
    }

    npc->final();
    tfp->close();

    IFDEF(CONFIG_FTRACE, close_log());

    // print hit good/fail and return value
    if (halt_valid == 1) {
        printf((halt_value == 0 ? ANSI_FMT("HIT GOOD TRAP\n", ANSI_FG_GREEN) : ANSI_FMT("HIT BAD TRAP\n", ANSI_FG_RED)));
        return halt_value;
    } else {
        printf(ANSI_FMT("HIT BAD TRAP\n", ANSI_FG_RED));
        return 1;
    }
}
