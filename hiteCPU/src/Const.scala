package hiteCPU

import chisel3._
import chisel3.util._
import scala.math

object Config {
  val XLEN = 32
  // val START_ADDR = 0x8000_0000L
  val START_ADDR = 0x7fff_fffcL

  val REG_NUM = 32
  val REG_ADDR_LEN = 5
}

object Const {

  val N = false.B
  val Y = true.B

  // pc_sel
  val PC_SEL_LENGTH = 3
  val PC_4   = 0.U(PC_SEL_LENGTH.W)
  val PC_ALU = 1.U(PC_SEL_LENGTH.W)
  val PC_0   = 2.U(PC_SEL_LENGTH.W)
  val PC_EPC = 3.U(PC_SEL_LENGTH.W)
  val PC_BR  = 4.U(PC_SEL_LENGTH.W)

  // A_sel
  val A_SEL_LENGTH = 1
  val A_XXX = 0.U(A_SEL_LENGTH.W)
  val A_PC  = 0.U(A_SEL_LENGTH.W)
  val A_RS1 = 1.U(A_SEL_LENGTH.W)

  // B_sel
  val B_SEL_LENGTH = 1
  val B_XXX = 0.U(B_SEL_LENGTH.W)
  val B_IMM = 0.U(B_SEL_LENGTH.W)
  val B_RS2 = 1.U(B_SEL_LENGTH.W)

  // imm_sel
  val IMM_SEL_LENGTH = 3
  val IMM_X = 0.U(IMM_SEL_LENGTH.W)
  val IMM_I = 1.U(IMM_SEL_LENGTH.W)
  val IMM_S = 2.U(IMM_SEL_LENGTH.W)
  val IMM_U = 3.U(IMM_SEL_LENGTH.W)
  val IMM_J = 4.U(IMM_SEL_LENGTH.W)
  val IMM_B = 5.U(IMM_SEL_LENGTH.W)
  val IMM_Z = 6.U(IMM_SEL_LENGTH.W)

  // alu_sel
  val ALU_SEL_LENGTH = 4
  val ALU_XXX    = 0.U(ALU_SEL_LENGTH.W)
  val ALU_ADD    = 0.U(ALU_SEL_LENGTH.W)
  val ALU_SLL    = 1.U(ALU_SEL_LENGTH.W)
  val ALU_SEQ    = 2.U(ALU_SEL_LENGTH.W)
  val ALU_SNE    = 3.U(ALU_SEL_LENGTH.W)
  val ALU_XOR    = 4.U(ALU_SEL_LENGTH.W)
  val ALU_SRL    = 5.U(ALU_SEL_LENGTH.W)
  val ALU_OR     = 6.U(ALU_SEL_LENGTH.W)
  val ALU_AND    = 7.U(ALU_SEL_LENGTH.W)
  val ALU_COPY_A = 8.U(ALU_SEL_LENGTH.W)
  val ALU_COPY_B = 9.U(ALU_SEL_LENGTH.W)
  val ALU_SUB    = 10.U(ALU_SEL_LENGTH.W)
  val ALU_SRA    = 11.U(ALU_SEL_LENGTH.W)
  val ALU_SLT    = 12.U(ALU_SEL_LENGTH.W)
  val ALU_SGE    = 13.U(ALU_SEL_LENGTH.W)
  val ALU_SLTU   = 14.U(ALU_SEL_LENGTH.W)
  val ALU_SGEU   = 15.U(ALU_SEL_LENGTH.W)

  def isSub(alu_op: UInt) = alu_op(3)
  def shiftSigned(alu_op: UInt) = alu_op(3)
  def isCmp(alu_op: UInt) = alu_op >= ALU_SLT
  def cmpUnsigned(alu_op: UInt) = alu_op(1)
  def cmpInverted(alu_op: UInt) = alu_op(0)
  def cmpEq(alu_op: UInt) = !alu_op(3)

  // st_type
  val ST_SEL_LENGTH = 2
  val ST_XXX = 0.U(ST_SEL_LENGTH.W)
  val ST_SW  = 1.U(ST_SEL_LENGTH.W)
  val ST_SH  = 2.U(ST_SEL_LENGTH.W)
  val ST_SB  = 3.U(ST_SEL_LENGTH.W)

  // ld_type
  val LD_SEL_LENGTH = 3
  val LD_XXX = 0.U(LD_SEL_LENGTH.W)
  val LD_LW  = 1.U(LD_SEL_LENGTH.W)
  val LD_LH  = 2.U(LD_SEL_LENGTH.W)
  val LD_LB  = 3.U(LD_SEL_LENGTH.W)
  val LD_LHU = 4.U(LD_SEL_LENGTH.W)
  val LD_LBU = 5.U(LD_SEL_LENGTH.W)

  // wb_sel
  val WB_SEL_LENGTH = 2
  val WB_XXX = 0.U(WB_SEL_LENGTH.W)
  val WB_ALU = 0.U(WB_SEL_LENGTH.W)
  val WB_MEM = 1.U(WB_SEL_LENGTH.W)
  val WB_PC4 = 2.U(WB_SEL_LENGTH.W)
  val WB_CSR = 3.U(WB_SEL_LENGTH.W)

  //csr_cmd
  val CSR_SEL_LENGTH = 3
  val CSR_N = 0.U(CSR_SEL_LENGTH.W)
  val CSR_W = 1.U(CSR_SEL_LENGTH.W)
  val CSR_S = 2.U(CSR_SEL_LENGTH.W)
  val CSR_C = 3.U(CSR_SEL_LENGTH.W)
  val CSR_P = 4.U(CSR_SEL_LENGTH.W)

}

object Instructions {
  // Loads
  def LB  = BitPat("b?????????????????000?????0000011")
  def LH  = BitPat("b?????????????????001?????0000011")
  def LW  = BitPat("b?????????????????010?????0000011")
  def LBU = BitPat("b?????????????????100?????0000011")
  def LHU = BitPat("b?????????????????101?????0000011")
  // Stores
  def SB = BitPat("b?????????????????000?????0100011")
  def SH = BitPat("b?????????????????001?????0100011")
  def SW = BitPat("b?????????????????010?????0100011")
  // Shifts
  def SLL  = BitPat("b0000000??????????001?????0110011")
  def SLLI = BitPat("b0000000??????????001?????0010011")
  def SRL  = BitPat("b0000000??????????101?????0110011")
  def SRLI = BitPat("b0000000??????????101?????0010011")
  def SRA  = BitPat("b0100000??????????101?????0110011")
  def SRAI = BitPat("b0100000??????????101?????0010011")
  // Arithmetic
  def ADD   = BitPat("b0000000??????????000?????0110011")
  def ADDI  = BitPat("b?????????????????000?????0010011")
  def SUB   = BitPat("b0100000??????????000?????0110011")
  def LUI   = BitPat("b?????????????????????????0110111")
  def AUIPC = BitPat("b?????????????????????????0010111")
  // Logical
  def XOR  = BitPat("b0000000??????????100?????0110011")
  def XORI = BitPat("b?????????????????100?????0010011")
  def OR   = BitPat("b0000000??????????110?????0110011")
  def ORI  = BitPat("b?????????????????110?????0010011")
  def AND  = BitPat("b0000000??????????111?????0110011")
  def ANDI = BitPat("b?????????????????111?????0010011")
  // Compare
  def SLT   = BitPat("b0000000??????????010?????0110011")
  def SLTI  = BitPat("b?????????????????010?????0010011")
  def SLTU  = BitPat("b0000000??????????011?????0110011")
  def SLTIU = BitPat("b?????????????????011?????0010011")
  // Branches
  def BEQ  = BitPat("b?????????????????000?????1100011")
  def BNE  = BitPat("b?????????????????001?????1100011")
  def BLT  = BitPat("b?????????????????100?????1100011")
  def BGE  = BitPat("b?????????????????101?????1100011")
  def BLTU = BitPat("b?????????????????110?????1100011")
  def BGEU = BitPat("b?????????????????111?????1100011")
  // Jump & Link
  def JAL = BitPat("b?????????????????????????1101111")
  def JALR = BitPat("b?????????????????000?????1100111")
  // Synch
  def FENCE = BitPat("b0000????????00000000000000001111")
  def FENCEI = BitPat("b00000000000000000001000000001111")
  // CSR Access
  def CSRRW = BitPat("b?????????????????001?????1110011")
  def CSRRS = BitPat("b?????????????????010?????1110011")
  def CSRRC = BitPat("b?????????????????011?????1110011")
  def CSRRWI = BitPat("b?????????????????101?????1110011")
  def CSRRSI = BitPat("b?????????????????110?????1110011")
  def CSRRCI = BitPat("b?????????????????111?????1110011")
  // Change Level
  def ECALL = BitPat("b00000000000000000000000001110011")
  def EBREAK = BitPat("b00000000000100000000000001110011")
  def ERET = BitPat("b00010000000000000000000001110011")
  def WFI = BitPat("b00010000001000000000000001110011")

}
