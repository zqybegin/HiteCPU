package hiteCPU

import chisel3._
import chisel3.util.HasBlackBoxInline

import Config._

class Halt extends BlackBox with HasBlackBoxInline {
  val io = IO(new Bundle {
    val valid = Input(Bool())
    val value = Input(UInt(XLEN.W))
  })
  setInline("Halt.v",
    """module Halt(
      |    input valid,
      |    input [31:0] value
      |    /* verilator lint_off NULLPORT */
      |);
      |
      |export "DPI-C" task ebreak;
      |task ebreak;
      |   output int halt_valid;
      |   output int halt_value;
      |   halt_valid = {31'b0,valid};
      |   halt_value = value;
      |endtask
      |
      |endmodule
    """.stripMargin)
}
