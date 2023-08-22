package hiteCPU

import chisel3._
import chisel3.util._

import Const._
import Config._

class AluPort extends Bundle {
  val src1 = Input(UInt(XLEN.W))
  val src2 = Input(UInt(XLEN.W))
  val op   = Input(UInt(ALU_LENGTH.W))
  val out  = Output(UInt(XLEN.W))
}

class Alu extends RawModule {
  val io = IO(new AluPort)

  // ADD, SUB
  val in2_inv = Mux(io.op === ALU_SUB, ~io.src2, io.src2)
  val add_out = io.src1 + in2_inv + (io.op === ALU_SUB)

  io.out := Mux1H(
    Seq (
      (io.op === ALU_ADD   ) -> add_out,
      (io.op === ALU_SUB   ) -> add_out,
      (io.op === ALU_COPY_B) -> io.src2,
    )
  )

}
