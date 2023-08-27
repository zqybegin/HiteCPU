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
      /*                                                                                  wb_en  illegal?
                     pc_sel  A_sel   B_sel  imm_sel   alu_op  halt st_type ld_type wb_sel  | csr_cmd |
                       |       |       |     |          |       |     |       |       |    |  |      |    */
      List(           PC_4  , A_XXX,  B_XXX, IMM_X, ALU_XXX   , N, ST_XXX, LD_XXX, WB_XXX, N, CSR_N, Y),
      Array(
        LUI   -> List(PC_4  , A_XXX,  B_IMM, IMM_U, ALU_COPY_B, N, ST_XXX, LD_XXX, WB_ALU, Y, CSR_N, N),
        AUIPC -> List(PC_4  , A_PC,   B_IMM, IMM_U, ALU_ADD   , N, ST_XXX, LD_XXX, WB_ALU, Y, CSR_N, N),
        JAL   -> List(PC_ALU, A_PC,   B_IMM, IMM_J, ALU_ADD   , N, ST_XXX, LD_XXX, WB_PC4, Y, CSR_N, N),
        JALR  -> List(PC_ALU, A_RS1,  B_IMM, IMM_I, ALU_ADD   , N, ST_XXX, LD_XXX, WB_PC4, Y, CSR_N, N),
        BEQ   -> List(PC_BR , A_RS1,  B_RS2, IMM_B, ALU_SEQ   , N, ST_XXX, LD_XXX, WB_XXX, N, CSR_N, N),
        BNE   -> List(PC_BR , A_RS1,  B_RS2, IMM_B, ALU_SNE   , N, ST_XXX, LD_XXX, WB_XXX, N, CSR_N, N),
        BLT   -> List(PC_BR , A_RS1,  B_RS2, IMM_B, ALU_SLT   , N, ST_XXX, LD_XXX, WB_XXX, N, CSR_N, N),
        BGE   -> List(PC_BR , A_RS1,  B_RS2, IMM_B, ALU_SGE   , N, ST_XXX, LD_XXX, WB_XXX, N, CSR_N, N),
        BLTU  -> List(PC_BR , A_RS1,  B_RS2, IMM_B, ALU_SLTU  , N, ST_XXX, LD_XXX, WB_XXX, N, CSR_N, N),
        BGEU  -> List(PC_BR , A_RS1,  B_RS2, IMM_B, ALU_SGEU  , N, ST_XXX, LD_XXX, WB_XXX, N, CSR_N, N),
        // LB    -> List(PC_0  , A_RS1,  B_IMM, IMM_I, ALU_ADD   , Y, ST_XXX, LD_LB , WB_MEM, Y, CSR_N, N),
        // LH    -> List(PC_0  , A_RS1,  B_IMM, IMM_I, ALU_ADD   , Y, ST_XXX, LD_LH , WB_MEM, Y, CSR_N, N),
        // LW    -> List(PC_0  , A_RS1,  B_IMM, IMM_I, ALU_ADD   , Y, ST_XXX, LD_LW , WB_MEM, Y, CSR_N, N),
        // LBU   -> List(PC_0  , A_RS1,  B_IMM, IMM_I, ALU_ADD   , Y, ST_XXX, LD_LBU, WB_MEM, Y, CSR_N, N),
        // LHU   -> List(PC_0  , A_RS1,  B_IMM, IMM_I, ALU_ADD   , Y, ST_XXX, LD_LHU, WB_MEM, Y, CSR_N, N),
        // SB    -> List(PC_4  , A_RS1,  B_IMM, IMM_S, ALU_ADD   , N, ST_SB , LD_XXX, WB_ALU, N, CSR_N, N),
        // SH    -> List(PC_4  , A_RS1,  B_IMM, IMM_S, ALU_ADD   , N, ST_SH , LD_XXX, WB_ALU, N, CSR_N, N),
        // SW    -> List(PC_4  , A_RS1,  B_IMM, IMM_S, ALU_ADD   , N, ST_SW , LD_XXX, WB_ALU, N, CSR_N, N),
        ADDI  -> List(PC_4  , A_RS1,  B_IMM, IMM_I, ALU_ADD   , N, ST_XXX, LD_XXX, WB_ALU, Y, CSR_N, N),
        SLTI  -> List(PC_4  , A_RS1,  B_IMM, IMM_I, ALU_SLT   , N, ST_XXX, LD_XXX, WB_ALU, Y, CSR_N, N),
        SLTIU -> List(PC_4  , A_RS1,  B_IMM, IMM_I, ALU_SLTU  , N, ST_XXX, LD_XXX, WB_ALU, Y, CSR_N, N),
        XORI  -> List(PC_4  , A_RS1,  B_IMM, IMM_I, ALU_XOR   , N, ST_XXX, LD_XXX, WB_ALU, Y, CSR_N, N),
        ORI   -> List(PC_4  , A_RS1,  B_IMM, IMM_I, ALU_OR    , N, ST_XXX, LD_XXX, WB_ALU, Y, CSR_N, N),
        ANDI  -> List(PC_4  , A_RS1,  B_IMM, IMM_I, ALU_AND   , N, ST_XXX, LD_XXX, WB_ALU, Y, CSR_N, N),
        SLLI  -> List(PC_4  , A_RS1,  B_IMM, IMM_I, ALU_SLL   , N, ST_XXX, LD_XXX, WB_ALU, Y, CSR_N, N),
        SRLI  -> List(PC_4  , A_RS1,  B_IMM, IMM_I, ALU_SRL   , N, ST_XXX, LD_XXX, WB_ALU, Y, CSR_N, N),
        SRAI  -> List(PC_4  , A_RS1,  B_IMM, IMM_I, ALU_SRA   , N, ST_XXX, LD_XXX, WB_ALU, Y, CSR_N, N),
        ADD   -> List(PC_4  , A_RS1,  B_RS2, IMM_X, ALU_ADD   , N, ST_XXX, LD_XXX, WB_ALU, Y, CSR_N, N),
        SUB   -> List(PC_4  , A_RS1,  B_RS2, IMM_X, ALU_SUB   , N, ST_XXX, LD_XXX, WB_ALU, Y, CSR_N, N),
        SLL   -> List(PC_4  , A_RS1,  B_RS2, IMM_X, ALU_SLL   , N, ST_XXX, LD_XXX, WB_ALU, Y, CSR_N, N),
        SLT   -> List(PC_4  , A_RS1,  B_RS2, IMM_X, ALU_SLT   , N, ST_XXX, LD_XXX, WB_ALU, Y, CSR_N, N),
        SLTU  -> List(PC_4  , A_RS1,  B_RS2, IMM_X, ALU_SLTU  , N, ST_XXX, LD_XXX, WB_ALU, Y, CSR_N, N),
        XOR   -> List(PC_4  , A_RS1,  B_RS2, IMM_X, ALU_XOR   , N, ST_XXX, LD_XXX, WB_ALU, Y, CSR_N, N),
        SRL   -> List(PC_4  , A_RS1,  B_RS2, IMM_X, ALU_SRL   , N, ST_XXX, LD_XXX, WB_ALU, Y, CSR_N, N),
        SRA   -> List(PC_4  , A_RS1,  B_RS2, IMM_X, ALU_SRA   , N, ST_XXX, LD_XXX, WB_ALU, Y, CSR_N, N),
        OR    -> List(PC_4  , A_RS1,  B_RS2, IMM_X, ALU_OR    , N, ST_XXX, LD_XXX, WB_ALU, Y, CSR_N, N),
        AND   -> List(PC_4  , A_RS1,  B_RS2, IMM_X, ALU_AND   , N, ST_XXX, LD_XXX, WB_ALU, Y, CSR_N, N),
        // FENCE -> List(PC_4  , A_XXX,  B_XXX, IMM_X, ALU_XXX   , N, ST_XXX, LD_XXX, WB_ALU, N, CSR_N, N),
        // FENCEI-> List(PC_0  , A_XXX,  B_XXX, IMM_X, ALU_XXX   , Y, ST_XXX, LD_XXX, WB_ALU, N, CSR_N, N),
        // CSRRW -> List(PC_0  , A_RS1,  B_XXX, IMM_X, ALU_COPY_A, Y, ST_XXX, LD_XXX, WB_CSR, Y, CSR_W, N),
        // CSRRS -> List(PC_0  , A_RS1,  B_XXX, IMM_X, ALU_COPY_A, Y, ST_XXX, LD_XXX, WB_CSR, Y, CSR_S, N),
        // CSRRC -> List(PC_0  , A_RS1,  B_XXX, IMM_X, ALU_COPY_A, Y, ST_XXX, LD_XXX, WB_CSR, Y, CSR_C, N),
        // CSRRWI-> List(PC_0  , A_XXX,  B_XXX, IMM_Z, ALU_XXX   , Y, ST_XXX, LD_XXX, WB_CSR, Y, CSR_W, N),
        // CSRRSI-> List(PC_0  , A_XXX,  B_XXX, IMM_Z, ALU_XXX   , Y, ST_XXX, LD_XXX, WB_CSR, Y, CSR_S, N),
        // CSRRCI-> List(PC_0  , A_XXX,  B_XXX, IMM_Z, ALU_XXX   , Y, ST_XXX, LD_XXX, WB_CSR, Y, CSR_C, N),
        // ECALL -> List(PC_4  , A_XXX,  B_XXX, IMM_X, ALU_XXX   , N, ST_XXX, LD_XXX, WB_CSR, N, CSR_P, N),
        // ERET  -> List(PC_EPC, A_XXX,  B_XXX, IMM_X, ALU_XXX   , Y, ST_XXX, LD_XXX, WB_CSR, N, CSR_P, N),
        // WFI   -> List(PC_4  , A_XXX,  B_XXX, IMM_X, ALU_XXX   , N, ST_XXX, LD_XXX, WB_ALU, N, CSR_N, N),
        EBREAK-> List(PC_4  , A_XXX,  B_XXX, IMM_X, ALU_XXX   , Y, ST_XXX, LD_XXX, WB_CSR, N, CSR_P, N),
      ),
    )

  // Control signals for Fetch
  io.pc_sel  := signals(0)
  // Control signals for Execute
  io.A_sel   := signals(1)
  io.B_sel   := signals(2)
  io.imm_sel := signals(3)
  io.alu_op  := signals(4)
  // Control signals for WriteBack
  io.wb_en   := signals(9).asBool
  io.wb_sel  := signals(8)
  // Control signals for Halt
  io.halt    := signals(5)
}
