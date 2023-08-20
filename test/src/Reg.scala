package test

import chisel3._

class Reg extends Module {
  val io = IO(new Bundle {
    val in  = Input(UInt(12.W))
    val out_1 = Output(UInt(12.W))
    val out_2 = Output(UInt(12.W))
    val out_3 = Output(UInt(12.W))
    val out_4 = Output(UInt(12.W))
    val out_5 = Output(UInt(12.W))
  })

  //第一种寄存器声明方式, 此处的寄存器将会初始化为随机值
  val register_1 = Reg(UInt(12.W))
  register_1 := io.in + 1.U
  io.out_1 := register_1

  //第二种寄存器声明方式, 此处的寄存器将会初始化为0.U
  val register_2 = RegInit(50.U(12.W))
  register_2 := io.in + 1.U
  io.out_2 := register_2

  //第二种寄存器声明方式, 此处的寄存器将会初始化为0.U
  val register_3 = RegInit(UInt(12.W), 0.U)
  register_3 := io.in + 1.U
  io.out_3 := register_3

  //第三种更为简单的寄存器声明方式,此处的寄存器将会初始化为随机值
  io.out_4 := RegNext(io.in + 1.U)

  //第三种更为简单的寄存器声明方式,此处的寄存器将会初始化为0.U
  io.out_5 := RegNext(io.in + 1.U, 0.U(12.W))
}
