package hiteCPU

import chisel3._
import chisel3.util.HasBlackBoxInline

class Halt extends BlackBox with HasBlackBoxInline {
  val io = IO(new Bundle {
    val in = Input(Bool())
  })
  setInline("Halt.v",
    """module Halt(
      |    input in
      |    /* verilator lint_off NULLPORT */
      |);
      |
      |export "DPI-C" task ebreak;
      |task ebreak;
      |   output int a;
      |   a = {31'b0,in};
      |endtask
      |
      |endmodule
    """.stripMargin)
}
