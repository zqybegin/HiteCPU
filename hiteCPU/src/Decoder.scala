package hiteCPU

import chisel3._
import chisel3.util._

import Const._
import Config._
import Instructions._

class DecoderPort extends Bundle {
  val inst    = Input(UInt(XLEN.W))
  // Control signals for Fetch
  val pc_sel  = Output(UInt(PC_SEL_LENGTH.W))
  // Control signals for Execute
  val A_sel   = Output(UInt(A_SEL_LENGTH.W))
  val B_sel   = Output(UInt(B_SEL_LENGTH.W))
  val imm_sel = Output(UInt(IMM_SEL_LENGTH.W))
  val alu_op  = Output(UInt(ALU_SEL_LENGTH.W))
  // Control signals for Mem
  val mem_sel = Output(UInt(MEM_SEL_LENGTH.W))
  // Control signals for WriteBack
  val wb_en   = Output(Bool())
  val wb_sel  = Output(UInt(WB_SEL_LENGTH.W))
  // Control signals for Halt
  val halt    = Output(Bool())
}

class Decoder extends RawModule {
  val io = IO(new DecoderPort)

  val signals =
    ListLookup(
      io.inst,
      /*                                                                           wb_en  illegal?
                     pc_sel  A_sel   B_sel  imm_sel   alu_op    st_type  wb_sel  | csr_cmd | halt
                       |       |       |     |          |          |        |    |  |      |  |  */
      List(           PC_4  , A_XXX,  B_XXX, IMM_X, ALU_XXX   , MEM_XXX, WB_XXX, N, CSR_N, Y, N),
      Array(
        LUI   -> List(PC_4  , A_XXX,  B_IMM, IMM_U, ALU_COPY_B, MEM_XXX, WB_ALU, Y, CSR_N, N, N),
        AUIPC -> List(PC_4  , A_PC,   B_IMM, IMM_U, ALU_ADD   , MEM_XXX, WB_ALU, Y, CSR_N, N, N),
        JAL   -> List(PC_ALU, A_PC,   B_IMM, IMM_J, ALU_ADD   , MEM_XXX, WB_PC4, Y, CSR_N, N, N),
        JALR  -> List(PC_ALU, A_RS1,  B_IMM, IMM_I, ALU_ADD   , MEM_XXX, WB_PC4, Y, CSR_N, N, N),
        BEQ   -> List(PC_BR , A_RS1,  B_RS2, IMM_B, ALU_SEQ   , MEM_XXX, WB_XXX, N, CSR_N, N, N),
        BNE   -> List(PC_BR , A_RS1,  B_RS2, IMM_B, ALU_SNE   , MEM_XXX, WB_XXX, N, CSR_N, N, N),
        BLT   -> List(PC_BR , A_RS1,  B_RS2, IMM_B, ALU_SLT   , MEM_XXX, WB_XXX, N, CSR_N, N, N),
        BGE   -> List(PC_BR , A_RS1,  B_RS2, IMM_B, ALU_SGE   , MEM_XXX, WB_XXX, N, CSR_N, N, N),
        BLTU  -> List(PC_BR , A_RS1,  B_RS2, IMM_B, ALU_SLTU  , MEM_XXX, WB_XXX, N, CSR_N, N, N),
        BGEU  -> List(PC_BR , A_RS1,  B_RS2, IMM_B, ALU_SGEU  , MEM_XXX, WB_XXX, N, CSR_N, N, N),
        LB    -> List(PC_4  , A_RS1,  B_IMM, IMM_I, ALU_ADD   , MEM_LB , WB_MEM, Y, CSR_N, N, N),
        LH    -> List(PC_4  , A_RS1,  B_IMM, IMM_I, ALU_ADD   , MEM_LH , WB_MEM, Y, CSR_N, N, N),
        LW    -> List(PC_4  , A_RS1,  B_IMM, IMM_I, ALU_ADD   , MEM_LW , WB_MEM, Y, CSR_N, N, N),
        LBU   -> List(PC_4  , A_RS1,  B_IMM, IMM_I, ALU_ADD   , MEM_LBU, WB_MEM, Y, CSR_N, N, N),
        LHU   -> List(PC_4  , A_RS1,  B_IMM, IMM_I, ALU_ADD   , MEM_LHU, WB_MEM, Y, CSR_N, N, N),
        SB    -> List(PC_4  , A_RS1,  B_IMM, IMM_S, ALU_ADD   , MEM_SB , WB_ALU, N, CSR_N, N, N),
        SH    -> List(PC_4  , A_RS1,  B_IMM, IMM_S, ALU_ADD   , MEM_SH , WB_ALU, N, CSR_N, N, N),
        SW    -> List(PC_4  , A_RS1,  B_IMM, IMM_S, ALU_ADD   , MEM_SW , WB_ALU, N, CSR_N, N, N),
        ADDI  -> List(PC_4  , A_RS1,  B_IMM, IMM_I, ALU_ADD   , MEM_XXX, WB_ALU, Y, CSR_N, N, N),
        SLTI  -> List(PC_4  , A_RS1,  B_IMM, IMM_I, ALU_SLT   , MEM_XXX, WB_ALU, Y, CSR_N, N, N),
        SLTIU -> List(PC_4  , A_RS1,  B_IMM, IMM_I, ALU_SLTU  , MEM_XXX, WB_ALU, Y, CSR_N, N, N),
        XORI  -> List(PC_4  , A_RS1,  B_IMM, IMM_I, ALU_XOR   , MEM_XXX, WB_ALU, Y, CSR_N, N, N),
        ORI   -> List(PC_4  , A_RS1,  B_IMM, IMM_I, ALU_OR    , MEM_XXX, WB_ALU, Y, CSR_N, N, N),
        ANDI  -> List(PC_4  , A_RS1,  B_IMM, IMM_I, ALU_AND   , MEM_XXX, WB_ALU, Y, CSR_N, N, N),
        SLLI  -> List(PC_4  , A_RS1,  B_IMM, IMM_I, ALU_SLL   , MEM_XXX, WB_ALU, Y, CSR_N, N, N),
        SRLI  -> List(PC_4  , A_RS1,  B_IMM, IMM_I, ALU_SRL   , MEM_XXX, WB_ALU, Y, CSR_N, N, N),
        SRAI  -> List(PC_4  , A_RS1,  B_IMM, IMM_I, ALU_SRA   , MEM_XXX, WB_ALU, Y, CSR_N, N, N),
        ADD   -> List(PC_4  , A_RS1,  B_RS2, IMM_X, ALU_ADD   , MEM_XXX, WB_ALU, Y, CSR_N, N, N),
        SUB   -> List(PC_4  , A_RS1,  B_RS2, IMM_X, ALU_SUB   , MEM_XXX, WB_ALU, Y, CSR_N, N, N),
        SLL   -> List(PC_4  , A_RS1,  B_RS2, IMM_X, ALU_SLL   , MEM_XXX, WB_ALU, Y, CSR_N, N, N),
        SLT   -> List(PC_4  , A_RS1,  B_RS2, IMM_X, ALU_SLT   , MEM_XXX, WB_ALU, Y, CSR_N, N, N),
        SLTU  -> List(PC_4  , A_RS1,  B_RS2, IMM_X, ALU_SLTU  , MEM_XXX, WB_ALU, Y, CSR_N, N, N),
        XOR   -> List(PC_4  , A_RS1,  B_RS2, IMM_X, ALU_XOR   , MEM_XXX, WB_ALU, Y, CSR_N, N, N),
        SRL   -> List(PC_4  , A_RS1,  B_RS2, IMM_X, ALU_SRL   , MEM_XXX, WB_ALU, Y, CSR_N, N, N),
        SRA   -> List(PC_4  , A_RS1,  B_RS2, IMM_X, ALU_SRA   , MEM_XXX, WB_ALU, Y, CSR_N, N, N),
        OR    -> List(PC_4  , A_RS1,  B_RS2, IMM_X, ALU_OR    , MEM_XXX, WB_ALU, Y, CSR_N, N, N),
        AND   -> List(PC_4  , A_RS1,  B_RS2, IMM_X, ALU_AND   , MEM_XXX, WB_ALU, Y, CSR_N, N, N),
        // FENCE -> List(PC_4  , A_XXX,  B_XXX, IMM_X, ALU_XXX   , MEM_XXX, WB_ALU, N, CSR_N, N, N),
        // FENCEI-> List(PC_0  , A_XXX,  B_XXX, IMM_X, ALU_XXX   , MEM_XXX, WB_ALU, N, CSR_N, N, N),
        // CSRRW -> List(PC_0  , A_RS1,  B_XXX, IMM_X, ALU_COPY_A, MEM_XXX, WB_CSR, Y, CSR_W, N, N),
        // CSRRS -> List(PC_0  , A_RS1,  B_XXX, IMM_X, ALU_COPY_A, MEM_XXX, WB_CSR, Y, CSR_S, N, N),
        // CSRRC -> List(PC_0  , A_RS1,  B_XXX, IMM_X, ALU_COPY_A, MEM_XXX, WB_CSR, Y, CSR_C, N, N),
        // CSRRWI-> List(PC_0  , A_XXX,  B_XXX, IMM_Z, ALU_XXX   , MEM_XXX, WB_CSR, Y, CSR_W, N, N),
        // CSRRSI-> List(PC_0  , A_XXX,  B_XXX, IMM_Z, ALU_XXX   , MEM_XXX, WB_CSR, Y, CSR_S, N, N),
        // CSRRCI-> List(PC_0  , A_XXX,  B_XXX, IMM_Z, ALU_XXX   , MEM_XXX, WB_CSR, Y, CSR_C, N, N),
        // ECALL -> List(PC_4  , A_XXX,  B_XXX, IMM_X, ALU_XXX   , MEM_XXX, WB_CSR, N, CSR_P, N, N),
        // ERET  -> List(PC_EPC, A_XXX,  B_XXX, IMM_X, ALU_XXX   , MEM_XXX, WB_CSR, N, CSR_P, N, N),
        // WFI   -> List(PC_4  , A_XXX,  B_XXX, IMM_X, ALU_XXX   , MEM_XXX, WB_ALU, N, CSR_N, N, N),
        EBREAK-> List(PC_4  , A_XXX,  B_XXX, IMM_X, ALU_XXX   , MEM_XXX, WB_CSR, N, CSR_P, N, Y),
      ),
    )

  // Control signals for Fetch
  io.pc_sel  := signals(0)
  // Control signals for Execute
  io.A_sel   := signals(1)
  io.B_sel   := signals(2)
  io.imm_sel := signals(3)
  io.alu_op  := signals(4)
  // Control signals for Mem
  io.mem_sel := signals(5)
  // Control signals for WriteBack
  io.wb_en   := signals(7).asBool
  io.wb_sel  := signals(6)
  // Control signals for Halt
  io.halt    := signals(10)
}
