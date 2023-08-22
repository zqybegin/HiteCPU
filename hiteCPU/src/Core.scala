package hiteCPU

import chisel3._
import chisel3.util._

import Config._
import Const._

class CoreIO extends Bundle {
  val mem = new MemPort
  val test = Flipped(new RegWrite)
}

class Core extends Module {
  val io = IO(new CoreIO)

  // --------------- Fetch Stage --------------- //
  val pc = RegInit(START_ADDR.U(XLEN.W))
  // val pc = RegInit(UInt(XLEN.W), START_ADDR.U)
  pc := pc + 4.U


  // --------------- Decode Stage --------------- //
  val inst = io.mem.resp.data

  val decoder = Module(new Decoder)
  decoder.io.inst := io.mem.resp.data

  val regfile= Module(new RegFile)
  regfile.io.read1.addr := io.mem.resp.data(19,15)
  regfile.io.read2.addr := io.mem.resp.data(24,20)
  regfile.io.halt.addr  := 10.U


  val imm = Mux1H(
    Seq (
      (decoder.io.imm_sel === IMM_I) -> inst(31, 20).asSInt,
      (decoder.io.imm_sel === IMM_S) -> Cat(inst(31, 25), inst(11, 7)).asSInt,
      (decoder.io.imm_sel === IMM_U) -> Cat(inst(31, 12), 0.U(12.W)).asSInt,
      (decoder.io.imm_sel === IMM_J) -> Cat(inst(31), inst(19, 12), inst(20), inst(30, 25), inst(24, 21), 0.U(1.W)).asSInt,
      (decoder.io.imm_sel === IMM_B) -> Cat(inst(31), inst(7), inst(30, 25), inst(11, 8), 0.U(1.W)).asSInt,
    )
  )

  val opdata1 = Mux1H(
    Seq (
      (decoder.io.A_sel === A_RS1) -> regfile.io.read1.data,
      (decoder.io.A_sel === A_PC)  -> 0.U,
    )
  )

  val opdata2 = Mux1H(
    Seq (
      (decoder.io.B_sel === B_RS2) -> regfile.io.read2.data,
      (decoder.io.B_sel === B_IMM) -> imm.asUInt
    )
  )

  // --------------- Execute Stage --------------- //
  val alu = Module(new Alu)
  alu.io.op := decoder.io.alu_op
  alu.io.src1 := opdata1
  alu.io.src2 := opdata2

  // --------------- Execute Stage --------------- //
  regfile.io.write.valid := decoder.io.wb_en
  regfile.io.write.addr  := inst(11,7)
  regfile.io.write.data  := alu.io.out

  // --------------- DPI-C halt --------------- //
  val halt = Module(new Halt)
  halt.io.valid := decoder.io.halt
  halt.io.value := regfile.io.halt.data

  // --------------- Top signal --------------- //
  io.mem.req.valid := !reset.asBool
  io.mem.req.bits.addr := pc + 4.U

  io.test.valid := decoder.io.wb_en
  io.test.addr  := inst(11,7)
  io.test.data  := alu.io.out

}
