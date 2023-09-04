package hiteCPU

import chisel3._
import chisel3.util._

import Config._
import Const._

class CoreIO extends Bundle {
  val imem = new MemPort
  val dmem = new MemPort
  val test = Flipped(new RegWrite)
}

class Core extends Module {
  val io = IO(new CoreIO)

  // --------------- Fetch Stage --------------- //
  val pc = RegInit(START_ADDR.U(XLEN.W))

  // --------------- Decode Stage --------------- //
  val inst = io.imem.resp.data

  val decoder = Module(new Decoder)
  decoder.io.inst := io.imem.resp.data

  val regfile= Module(new RegFile)
  regfile.io.read1.addr := io.imem.resp.data(19,15)
  regfile.io.read2.addr := io.imem.resp.data(24,20)
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
      (decoder.io.A_sel === A_PC)  -> pc,
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

  // --------------- Mem Stage --------------- //
  io.dmem.req.valid     := ~decoder.io.mem_sel.andR
  io.dmem.req.bits.wr   := isStore(decoder.io.mem_sel)
  io.dmem.req.bits.size := getSize(decoder.io.mem_sel)
  io.dmem.req.bits.addr := alu.io.add_out
  io.dmem.req.bits.data := regfile.io.read2.data


  val load_data = Mux1H(
    Seq (
      (decoder.io.mem_sel === MEM_LW ) -> io.dmem.resp.data.zext,
      (decoder.io.mem_sel === MEM_LH ) -> io.dmem.resp.data(15,0).asSInt,
      (decoder.io.mem_sel === MEM_LB ) -> io.dmem.resp.data(7,0).asSInt,
      (decoder.io.mem_sel === MEM_LHU) -> io.dmem.resp.data(15,0).zext,
      (decoder.io.mem_sel === MEM_LBU) -> io.dmem.resp.data(7,0).zext,
    )
  )

  // --------------- WriteBack Stage --------------- //
  val wb_data = Wire(UInt(32.W))
  wb_data := Mux1H(
    Seq (
      (decoder.io.wb_sel === WB_ALU) -> alu.io.alu_out,
      (decoder.io.wb_sel === WB_PC4) -> (pc + 4.U),
      (decoder.io.wb_sel === WB_MEM) -> load_data.asUInt,
    )
  )

  val npc = Mux1H(
    Seq (
      (decoder.io.pc_sel === PC_4) -> (pc + 4.U),
      (decoder.io.pc_sel === PC_ALU) -> alu.io.add_out,
      (decoder.io.pc_sel === PC_BR) -> Mux(alu.io.cmp_out === 1.U, pc + imm.asUInt, pc + 4.U),
    )
  )
  pc := npc

  regfile.io.write.valid := decoder.io.wb_en
  regfile.io.write.addr  := inst(11,7)
  regfile.io.write.data  := wb_data

  // --------------- DPI-C halt --------------- //
  val halt = Module(new Halt)
  halt.io.valid := decoder.io.halt
  halt.io.value := regfile.io.halt.data

  // --------------- Top signal --------------- //
  io.imem.req.valid     := !reset.asBool
  io.imem.req.bits.wr   := false.B
  io.imem.req.bits.size := 2.U
  io.imem.req.bits.addr := npc
  io.imem.req.bits.data := 0.U

  io.test.valid := decoder.io.wb_en
  io.test.addr  := inst(11,7)
  io.test.data  := wb_data

}
