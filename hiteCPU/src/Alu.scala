package hiteCPU

import chisel3._
import chisel3.util._

import Const._
import Config._

class AluPort extends Bundle {
  val src1 = Input(UInt(XLEN.W))
  val src2 = Input(UInt(XLEN.W))
  val op   = Input(UInt(ALU_SEL_LENGTH.W))
  val alu_out = Output(UInt(XLEN.W))
  val cmp_out = Output(UInt(XLEN.W))
  val add_out = Output(UInt(XLEN.W))
}

class Alu extends RawModule {
  val io = IO(new AluPort)

  // ADD, SUB
  val in2_inv = Mux(isSub(io.op), ~io.src2, io.src2)
  val in1_xor_in2 = io.src1 ^ in2_inv
  io.add_out := io.src1 + in2_inv + isSub(io.op)

  // SLT, SLTU, EQ, NEQ, SGTU, SGT
  val slt =
    Mux(io.src1(XLEN-1) === io.src2(XLEN-1), io.add_out(XLEN-1),
      Mux(cmpUnsigned(io.op), io.src2(XLEN-1), io.src1(XLEN-1)))
  io.cmp_out := cmpInverted(io.op) ^ Mux(cmpEq(io.op), in1_xor_in2 === 0.U, slt)

  // SLL, SRL, SRA
  val (shamt, shin_r) = (io.src2(4,0), io.src1)
  val shin = Mux(io.op === ALU_SRL  || io.op === ALU_SRA, shin_r, Reverse(shin_r))
  val shout_r = (Cat(shiftSigned(io.op) & shin(XLEN-1), shin).asSInt >> shamt)(XLEN-1,0)
  val shout_l = Reverse(shout_r)
  val shout = Mux(io.op === ALU_SRL || io.op === ALU_SRA, shout_r, 0.U) |
              Mux(io.op === ALU_SLL,                      shout_l, 0.U)

  // AND, OR, XOR
  val logic = Mux(io.op === ALU_XOR || io.op === ALU_OR,  in1_xor_in2, 0.U) |
              Mux(io.op === ALU_OR  || io.op === ALU_AND, io.src1 & io.src2, 0.U)

  // AND, OR, XOR
  val copy = Mux(io.op === ALU_COPY_A,  io.src1, 0.U) |
             Mux(io.op === ALU_COPY_B,  io.src2, 0.U) ;


  // Select Alu ouput
  val out_temp = (isCmp(io.op) && slt) | logic | shout | copy
  val alu_out = Mux(io.op === ALU_ADD || io.op === ALU_SUB, io.add_out, out_temp)
  io.alu_out := alu_out
}
