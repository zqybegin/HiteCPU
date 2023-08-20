package test

import chisel3._
import chisel3.util._

class SimpleDecoder extends RawModule {
  val io = IO(new Bundle {
    val imm_sel = Input(UInt(2.W))
  })
  val imm = Mux1H(
    Seq (
      (io.imm_sel === 1.U) -> 1.U,
      (io.imm_sel === 2.U) -> 2.U,
      (io.imm_sel === 3.U) -> 3.U,
      (io.imm_sel === 4.U) -> 4.U,
    )
  )
}

object Toplevel {
  def main(args: Array[String]): Unit = {
    //generate Hello.v in directory generated
    emitVerilog( new SimpleDecoder, Array( "-e", "verilog", "--target-dir", "test/generated"))
    emitVerilog( new Reg, Array( "-e", "verilog", "--target-dir", "test/generated"))

    //print verilog code in ternimal
    //println(getVerilogString(new Toplevel(32)))

    //print HelloWorld! in ternimal
    println("Successfully generate verilog!") //scala's function, not chisel's function
  }
}
