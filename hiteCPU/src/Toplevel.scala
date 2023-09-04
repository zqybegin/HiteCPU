package hiteCPU

import chisel3._
import chisel3.util._

class ToplevelIO extends Bundle {
  val imem = new MemPort
  val dmem = new MemPort
  val test = Flipped(new RegWrite)
}

class Toplevel extends Module {
  val io = IO(new ToplevelIO)

  val core = Module(new Core)

  io.imem.req := core.io.imem.req
  core.io.imem.resp := io.imem.resp

  io.dmem.req := core.io.dmem.req
  core.io.dmem.resp := io.dmem.resp

  io.test := core.io.test
}

object Toplevel {
  def main(args: Array[String]): Unit = {
    //generate Hello.v in directory generated
    emitVerilog( new Toplevel, Array( "-e", "verilog", "--target-dir", "verilog"))

    //print verilog code in ternimal
    //println(getVerilogString(new Toplevel(32)))

    //print HelloWorld! in ternimal
    println("Successfully generate verilog!") //scala's function, not chisel's function
  }
}
