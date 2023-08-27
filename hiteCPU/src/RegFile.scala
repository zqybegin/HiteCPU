package hiteCPU

import chisel3._
import chisel3.util._
import chisel3.util.experimental.loadMemoryFromFile
import chisel3.util.experimental.loadMemoryFromFileInline

import Config._

class RegRead extends Bundle {
  val addr = Input(UInt(REG_ADDR_LEN.W))
  val data = Output(UInt(XLEN.W))
}

class RegWrite extends Bundle {
  val valid = Input(Bool())
  val addr  = Input(UInt(REG_ADDR_LEN.W))
  val data  = Input(UInt(XLEN.W))
}

class RegFilePort extends Bundle {
  val read1 = new RegRead
  val read2 = new RegRead
  val halt  = new RegRead
  val write = new RegWrite
}

class RegFile extends Module {
  val io = IO(new RegFilePort)
  val regs = Mem(REG_NUM, UInt(XLEN.W))

  loadMemoryFromFileInline(regs, "/home/zqybegin/Workstation/ysyx-workbench/npc/hiteCPU/src/regfile.Hex")

  io.read1.data := Mux(io.read1.addr.orR, regs(io.read1.addr), 0.U)
  io.read2.data := Mux(io.read2.addr.orR, regs(io.read2.addr), 0.U)
  io.halt.data := Mux(io.halt.addr.orR, regs(io.halt.addr), 0.U)

  when(io.write.valid && io.write.addr.orR) {
    regs(io.write.addr) := io.write.data
  }
}
