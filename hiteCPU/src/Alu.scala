package hiteCPU

import chisel3._
import chisel3.util._

import Const._
import Config._

class AluPort extends Bundle {
  val src1 = Input(UInt(XLEN.W))
  val src2 = Input(UInt(XLEN.W))
  val op   = Input(UInt(ALU_LENGTH.W))
  val out  = Output(UInt(ALU_LENGTH.W))
}

class Alu extends RawModule {
  val io = IO(new AluPort)

  // ADD, SUB
  val in2_inv = Mux(isSub(io.op), ~io.src2, io.src2)
  io.out := io.src1 + in2_inv + isSub(io.op)

}
