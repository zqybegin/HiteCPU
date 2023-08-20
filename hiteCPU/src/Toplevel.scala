package hiteCPU

import chisel3._
import chisel3.util._

class ToplevelIO extends Bundle {
  val mem = new MemPort
  val test = Flipped(new RegWrite)
}

class Toplevel extends Module {
  val io = IO(new ToplevelIO)

  val core = Module(new Core)

  io.mem.req := core.io.mem.req
  io.test := core.io.test
  core.io.mem.resp := io.mem.resp
}

object Toplevel {
  def main(args: Array[String]): Unit = {
    //generate Hello.v in directory generated
    emitVerilog( new Toplevel, Array( "-e", "verilog", "--target-dir", "generated"))

    //print verilog code in ternimal
    //println(getVerilogString(new Toplevel(32)))

    //print HelloWorld! in ternimal
    println("Successfully generate verilog!") //scala's function, not chisel's function
  }
}
