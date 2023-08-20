package hiteCPU

import chisel3._
import chisel3.util.Valid

import Config._

// mem req/resp port
class MemReq extends Bundle {
  val addr = Output(UInt(XLEN.W))
}

class MemResp extends Bundle {
  val data = Input(UInt(XLEN.W))
}

class MemPort extends Bundle {
  val req = Valid(new MemReq)
  val resp = new MemResp
}
