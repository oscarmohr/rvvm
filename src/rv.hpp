#ifndef RV_HPP
#define RV_HPP

#include "olib.hpp"
#include "bits.hpp"

namespace rvvm {

using namespace olib;


enum class Opcode {
  LUI    = 0b011'0111,
  AUIPC  = 0b001'0111,
  JAL    = 0b110'1111,
  JALR   = 0b110'0111,
  BRANCH = 0b110'0011,
  LOAD   = 0b000'0011,
  STORE  = 0b010'0011,
  OP_IMM = 0b001'0011,
  OP     = 0b011'0011,
  SYSTEM = 0b111'0011,
};

enum class rvInstructionType {
  R, I, S, B, U, J
};

enum class rvInstruction {
  LUI, AUIPC,                                           // type_U
  JAL,                                                  // type_J
  JALR,                                                 // type_I
  BEQ, BNE, BLT, BGE, BLTU, BGEU,                       // type_B
  LB, LH, LW, LBU, LHU,                                 // type_I
  SB, SH, SW,                                           // type_S
  ADDI, SLTI, SLTIU, XORI, ORI, ANDI, SLLI, SRLI, SRAI, // type_I
  ADD, SUB, SLL, SLT, SLTU, XOR, SRL, SRA, OR, AND,     // type_R
  FENCE, FENCE_TSO, ECALL, EBREAK
};

const map<Opcode, string> opcodeString {
  {Opcode::LUI,    "LUI"},
  {Opcode::AUIPC,  "AUIPC"},
  {Opcode::JAL,    "JAL"},
  {Opcode::JALR,   "JALR"},
  {Opcode::BRANCH, "BRANCH"},
  {Opcode::LOAD,   "LOAD"},
  {Opcode::STORE,  "STORE"},
  {Opcode::OP_IMM, "OP_IMM"},
  {Opcode::OP,     "OP"},
  {Opcode::SYSTEM, "SYSTEM"},
};

const map<rvInstruction, string> rvInstructionString {
  {rvInstruction::LUI,"LUI"},
  {rvInstruction::AUIPC,"AUIPC"},
  {rvInstruction::JAL,"JAL"},
  {rvInstruction::JALR,"JALR"},
  {rvInstruction::BEQ,"BEQ"},
  {rvInstruction::BNE,"BNE"},
  {rvInstruction::BLT,"BLT"},
  {rvInstruction::BGE,"BGE"},
  {rvInstruction::BLTU,"BLTU"},
  {rvInstruction::BGEU,"BGEU"},
  {rvInstruction::LB,"LB"},
  {rvInstruction::LH,"LH"},
  {rvInstruction::LW,"LW"},
  {rvInstruction::LBU,"LBU"},
  {rvInstruction::LHU,"LHU"},
  {rvInstruction::SB,"SB"},
  {rvInstruction::SH,"SH"},
  {rvInstruction::SW,"SW"},
  {rvInstruction::ADDI,"ADDI"},
  {rvInstruction::SLTI,"SLTI"},
  {rvInstruction::SLTIU,"SLTIU"},
  {rvInstruction::XORI,"XORI"},
  {rvInstruction::ORI,"ORI"},
  {rvInstruction::ANDI,"ANDI"},
  {rvInstruction::SLLI,"SLLI"},
  {rvInstruction::SRLI,"SRLI"},
  {rvInstruction::SRAI,"SRAI"},
  {rvInstruction::ADD,"ADD"},
  {rvInstruction::SUB,"SUB"},
  {rvInstruction::SLL,"SLL"},
  {rvInstruction::SLT,"SLT"},
  {rvInstruction::SLTU,"SLTU"},
  {rvInstruction::XOR,"XOR"},
  {rvInstruction::SRL,"SRL"},
  {rvInstruction::SRA,"SRA"},
  {rvInstruction::OR,"OR"},
  {rvInstruction::AND,"AND"},
  {rvInstruction::FENCE,"FENCE"},
  {rvInstruction::FENCE_TSO,"FENCE_TSO"},
  {rvInstruction::ECALL,"ECALL"},
  {rvInstruction::EBREAK,"EBREAK"},
};

enum class rvInstructionField {
  Opcode, rd, rs1, rs2, funct3, funct7, imm
};

class Instruction {
  using bad_instr = std::invalid_argument;

  u32 slice(u8 l, u8 u) {return rvvm::slice(word,l,u);}
  /* u32 slice(u8 l, u8 u) {return slice(l, u);} */
  u32 get_rd()     {return slice( 7, 11);}
  u32 get_rs1()    {return slice(15, 19);}
  u32 get_rs2()    {return slice(20, 24);}
  u32 get_funct3() {return slice(12, 14);}
  u32 get_funct7() {return slice(25, 31);}
  Opcode get_opcode() {

    auto switch_or = [&](auto val, auto vals, auto throwable) {
      for (auto v : vals)
        if (v == val)
          return v;
      throw throwable;
    };

    switch (auto opcode = Opcode(slice(0,6)); opcode) {
      using enum Opcode;
      case LUI:
      case AUIPC:
      case JAL:
      case JALR:
      case BRANCH:
      case LOAD:
      case STORE:
      case OP_IMM:
      case OP:
      case SYSTEM: return opcode;
      default: throw bad_instr("bad instr: bad opcode");
    }
  }
  auto get_type() {
    switch (opcode) {
      using enum Opcode;
      using enum rvInstructionType;
      case LUI:    return U;
      case AUIPC:  return U;
      case JAL:    return J;
      case JALR:   return I;
      case BRANCH: return B;
      case LOAD:   return I;
      case STORE:  return S;
      case OP_IMM: return I;
      case OP:     return R;
      case SYSTEM: return I;
      default: throw bad_instr("bad instr: bad opcode");
    }
  }
  auto get_imm() {
    auto type_I_imm = [&]{return slice(20, 31);};
    auto type_S_imm = [&]{
      return slice(25, 31) << 5 | slice(7, 11);
    };
    auto type_B_imm = [&]{
      return slice(31, 31) << 12
           | slice(25, 30) <<  5
           | slice( 8, 11) <<  1
           | slice( 7,  8) << 11;
    };
    auto type_U_imm = [&]{return slice(12, 31) << 12;};
    auto type_J_imm = [&]{
      // [0] = 0 (even number)
      return slice(31, 32) << 20
           | slice(21, 30) <<  1
           | slice(20, 21) << 11
           | slice(12, 19) << 12;
    };

    switch (instr_type) {
      using enum rvInstructionType;
      case I: return sxt(type_I_imm(), 12);
      case S: return sxt(type_S_imm(), 12);
      case B: return sxt(type_B_imm(), 13);
      case U: return sxt(type_U_imm(), 32);
      case J: return sxt(type_J_imm(), 21);
      default: throw bad_instr("bad instr: bad instr type");
    }
  }
  auto get_instruction() {
    auto decode_branch = [&]{
      switch (funct3) {
        using enum rvInstruction;
        case 0b000: return BEQ;
        case 0b001: return BNE;
        case 0b100: return BLT;
        case 0b101: return BGE;
        case 0b110: return BLTU;
        case 0b111: return BGEU;
        default: throw bad_instr("bad instr: bad funct3");
      }
    };
    auto decode_load = [&]{
      switch (funct3) {
        using enum rvInstruction;
        case 0b000: return LB;
        case 0b001: return LH;
        case 0b010: return LW;
        case 0b100: return LBU;
        case 0b101: return LHU;
        default: throw bad_instr("bad instr: bad funct3");
      }
    };
    auto decode_store = [&]{
      switch (funct3) {
        using enum rvInstruction;
        case 0b000: return SB;
        case 0b001: return SH;
        case 0b010: return SW;
        default: throw bad_instr("bad instr: bad funct3");
      }
    };
    auto decode_op_imm = [&]{
      switch (funct3) {
        using enum rvInstruction;
        case 0b000: return ADDI;
        case 0b010: return SLTI;
        case 0b011: return SLTIU;
        case 0b100: return XORI;
        case 0b110: return ORI;
        case 0b111: return ANDI;
        case 0b001: return SLLI;
        case 0b101: return funct7 ? SRAI : SRLI;
        default: throw bad_instr("bad instr: bad funct3");
      }
    };
    auto decode_op = [&]{
      switch (funct3) {
        using enum rvInstruction;
        case 0b000: return funct7 ? SUB : ADD;
        case 0b001: return SLL;
        case 0b010: return SLT;
        case 0b011: return SLTU;
        case 0b100: return XOR;
        case 0b101: return funct7 ? SRA : SRL;
        case 0b110: return OR;
        case 0b111: return AND;
        default: throw bad_instr("bad instr: bad funct3");
      }
    };

    switch (opcode) {
      using enum rvInstruction;
      case Opcode::LUI:    return LUI;
      case Opcode::AUIPC:  return AUIPC;
      case Opcode::JAL:    return JAL;
      case Opcode::JALR:   return JALR;
      case Opcode::BRANCH: return decode_branch();
      case Opcode::LOAD:   return decode_load();
      case Opcode::STORE:  return decode_store();
      case Opcode::OP_IMM: return decode_op_imm();
      case Opcode::OP:     return decode_op();
      case Opcode::SYSTEM: return imm ? EBREAK : ECALL;
      default: throw bad_instr("bad instr: bad opcode");
    }
  }

  string program_str;

public:
  u32 word;
  u32 rd;
  u32 rs1;
  u32 rs2;
  u32 imm;
  Opcode opcode;
  u32 funct3;
  u32 funct7;
  rvInstruction instr;
  rvInstructionType instr_type;

  Instruction(u32 i) {decode(i);}

  string str() {
    return program_str;
  }

  friend ostream& operator<<(ostream& os, Instruction i) {
    return os << i.program_str;
  }

  void decode(u32 i) {
    word       = i;
    opcode     = get_opcode();
    funct3     = get_funct3();
    funct7     = get_funct7();
    rd         = get_rd();
    rs1        = get_rs1();
    rs2        = get_rs2();
    instr      = get_instruction();
    instr_type = get_type();
  }

  void encode(string s) {
    program_str = s;
    // produce machine code word from abstract riscv instruction
    // hallo welt
  }

};

} // namespace rvvm

#endif // #ifndef RV_HPP
