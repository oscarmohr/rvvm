/***********************************************************
    ____
   / / /   _ ____   ____   ___ __ ___
  / / /   | '__\ \ / /\ \ / / '_ ` _ \
 / / /    | |   \ V /  \ V /| | | | | |
/_/_/     |_|    \_/    \_/ |_| |_| |_|


          rvvm -- RISCV-VM by @oscarmohr
          see github.com/oscarmohr/rvvm
          see riscv.org/technical/specifications

***********************************************************/

#include "olib.hpp"
using namespace olib;
namespace rvvm {

////////////////////////////////////////////////////////////
// RISC-V Opcodes, InstructionTypes, Instructions, Decode //
////////////////////////////////////////////////////////////

u32 slice(u32 w, u8 l, u8 u) { return (w << 31 - u) >> 31 - u + l; }
u32 ones        (u8 l, u8 u) { return slice(-1, l, u) << l;        } 
u32 zeroes      (u8 l, u8 u) { return ~ones(l, u);                 }

u32 set_slice(u32& w, u32 v, u8 l, u8 u) { // v[u-l:0] = w[u:l]
  return (w = (w & zeroes(l, u)) | ((v << l) & ones(l, u)));
}

u32 sxt(u32 w, u8 from) { // sign extend
  if (slice(w, from, from)) return set_slice(w, -1, from + 1, 31);
  else                      return set_slice(w,  0, from + 1, 31);
}

enum class Opcode {
  LUI         = 0b011'0111,
  AUIPC       = 0b001'0111,
  JAL         = 0b110'1111,
  JALR        = 0b110'0111,
  BRANCH      = 0b110'0011,
  LOAD        = 0b000'0011,
  STORE       = 0b010'0011,
  OP_IMM      = 0b001'0011,
  OP          = 0b011'0011,
  MISC_MEM    = 0b000'1111,
  SYSTEM      = 0b111'0011,
  RVVM_NOIMPL,
};

enum class InstructionType { R, I, S, B, U, J, };

enum class Instruction { // RISC-V Instruction Set
  LUI, AUIPC, JAL, JALR, BEQ, BNE, BLT, BGE, BLTU, BGEU,
  LB, LH, LW, LBU, LHU, SB, SH, SW, ADDI, SLTI, SLTIU,
  XORI, ORI, ANDI, SLLI, SRLI, SRAI, ADD, SUB,
  SLL, SLT, SLTU, XOR, SRL, SRA, OR, AND,
  FENCE, FENCE_TSO, PAUSE, ECALL, EBREAK, RVVM_ERR
};

Opcode opcode(u64 i) { return Opcode(slice(i, 0, 6)); }
u64 get_rd(u64 i)        { return slice(i,  7, 11); }
u64 get_rs1(u64 i)       { return slice(i, 15, 19); }
u64 get_rs2(u64 i)       { return slice(i, 20, 24); }
u64 get_funct3(u64 i)    { return slice(i, 12, 14); }
u64 get_funct7(u64 i)    { return slice(i, 25, 31); }

InstructionType get_type(u64 i) {
  switch (opcode(i)) {
  using enum Opcode;
  using enum InstructionType;
  case LUI:         return U;
  case AUIPC:       return U;
  case JAL:         return J;
  case JALR:        return I;
  case BRANCH:      return B;
  case LOAD:        return I;
  case STORE:       return S;
  case OP_IMM:      return I;
  case OP:          return R;
  case MISC_MEM:    return I;
  case SYSTEM:      return I;
  case RVVM_NOIMPL: return I;
  default:          return I;
  }
}

u64 get_imm(u64 i) {
  switch (get_type(i)) {
  using enum InstructionType;
  case I: return sxt((slice(i, 31, 20)      ), 12);
  case S: return sxt((slice(i, 25, 31) <<  5)
                   | (slice(i,  7, 11)      ), 12);
  case B: return sxt((slice(i, 31, 32) << 12)
                   | (slice(i, 25, 30) <<  5)
                   | (slice(i,  8, 11) <<  1)
                   | (slice(i,  7,  8) << 11), 13);
  case U: return sxt((slice(i, 31, 12) << 12), 32);
  case J: return sxt((slice(i, 31, 32) << 20)
                   | (slice(i, 21, 30) <<  1)
                   | (slice(i, 20, 21) << 11)
                   | (slice(i, 12, 19) << 12), 21);
  default: return 0;
  }
}

Instruction get_instruction(u64 i) {
  switch (opcode(i)) {
  using enum Instruction;
  case Opcode::LUI:   return LUI;
  case Opcode::AUIPC: return AUIPC;
  case Opcode::JAL:   return JAL;
  case Opcode::JALR:  return JALR;
  case Opcode::BRANCH:
    switch (get_funct3(i)) {
    case 0b000: return BEQ;
    case 0b001: return BNE;
    case 0b100: return BLT;
    case 0b101: return BGE;
    case 0b110: return BLTU;
    case 0b111: return BGEU;
    default:    return RVVM_ERR;
    }
  case Opcode::LOAD:
    switch (get_funct3(i)) {
    case 0b000: return LB;
    case 0b001: return LH;
    case 0b010: return LW;
    case 0b100: return LBU;
    case 0b101: return LHU;
    default:    return RVVM_ERR;
    }
  case Opcode::STORE:
    switch (get_funct3(i)) {
    case 0b000: return SB;
    case 0b001: return SH;
    case 0b010: return SW;
    default:    return RVVM_ERR;
    }
  case Opcode::OP_IMM:
    switch (get_funct3(i)) {
    case 0b000: return ADDI;
    case 0b010: return SLTI;
    case 0b011: return SLTIU;
    case 0b100: return XORI;
    case 0b110: return ORI;
    case 0b111: return ANDI;
    case 0b001: return SLLI;
    case 0b101: return get_funct7(i) ? SRAI : SRLI;
    default:    return RVVM_ERR;
    }
  case Opcode::OP:
    switch (get_funct3(i)) {
    case 0b000: return get_funct7(i) ? SUB : ADD;
    case 0b001: return SLL;
    case 0b010: return SLT;
    case 0b011: return SLTU;
    case 0b100: return XOR;
    case 0b101: return get_funct7(i) ? SRA : SRL;
    case 0b110: return OR;
    case 0b111: return AND;
    default:    return RVVM_ERR;
    }
  case Opcode::MISC_MEM: return RVVM_ERR;
  case Opcode::SYSTEM:   return get_imm(i) ? EBREAK : ECALL;
  default:               return RVVM_ERR;
  }
}


////////////////////////////////////////////////////////////
// RISC-V Virtual Machine //////////////////////////////////
////////////////////////////////////////////////////////////

struct Machine {
  std::map<u32, u8> mem;
  std::vector<u32>  regs = std::vector<u32>(33);
  u32 pc = 0;

  // registers
  u32& x(u8 i);

  u32 lb(u32 i);
  u32 lh(u32 i);
  u32 lw(u32 i);
  u32 ld(u32 i);
  void sb(u32 w, u32 i);
  void sh(u32 w, u32 i);
  void sw(u32 w, u32 i);
  void sd(u32 w, u32 i);

  // load/store n bytes starting from adress i
  u32 load(u8 n, u32 i);
  void store(u8 n, u32 w, u32 i);

  void exec(u32 instruction);
};

u32& Machine::x(u8 i) { return i ? regs.at(i) : (regs.at(32) = 0); }

// loads and stores little endian
u32 Machine::lb(u32 i) { return load(1, i); }
u32 Machine::lh(u32 i) { return load(2, i); }
u32 Machine::lw(u32 i) { return load(4, i); }
u32 Machine::ld(u32 i) { return load(8, i); }
void Machine::sb(u32 w, u32 i) { store(1, w, i); }
void Machine::sh(u32 w, u32 i) { store(2, w, i); }
void Machine::sw(u32 w, u32 i) { store(4, w, i); }
void Machine::sd(u32 w, u32 i) { store(8, w, i); }

// load n bytes starting from adress i into word
u32 Machine::load(u8 n, u32 i) {
  u32 out = 0;
  for (auto i : range_(min(n, 8)))
    set_slice(out, mem.at(i), i*8, i*8+7); 
  return out;
}

// store n bytes of w into i
void Machine::store(u8 n, u32 w, u32 i) {
  for (auto j : range_(min(n, 8)))
    mem.at(i + j) = slice(w, j*8, j*8+7);
}

void Machine::exec(u32 i) {
  u32& rd  = x(get_rd(i));
  u32& rs1 = x(get_rs1(i));
  u32& rs2 = x(get_rs2(i));
  u32  imm = get_imm(i); 
  switch (get_instruction(i)) {
  using enum Instruction;
  case LUI:   rd = imm;                         break;
  case AUIPC: pc += imm;                        break;
  case JAL:   rd = pc + imm; pc += imm;         break;
  case JALR:  rd = rs1 + imm;                   break;
  case BEQ:   pc += rs1 == rs2 ? imm: 0;        break; 
  case BNE:   pc += rs1 != rs2 ? imm: 0;        break;
  case BLT:   pc += rs1  < rs2 ? imm: 0;        break;
  case BGE:   pc += rs1 >= rs2 ? imm: 0;        break;
  case BLTU:  pc += rs1  < rs2 ? imm: 0;        break;
  case BGEU:  pc += rs1 >= rs2 ? imm: 0;        break;
  case LB:    rd = lb(rs1 + imm);               break;
  case LH:    rd = lh(rs1 + imm);               break;
  case LW:    rd = lw(rs1 + imm);               break;
  case LBU:   rd = lb(rs1 + imm);               break;
  case LHU:   rd = lh(rs1 + imm);               break;
  case SB:    sb(rs1 + imm, slice(rs2, 0,  7)); break;
  case SH:    sh(rs1 + imm, slice(rs2, 0, 15)); break;
  case SW:    sw(rs1 + imm, rs2);               break;
  case ADDI:  rd = rs1  + imm;                  break;
  case SLTI:  rd = rs1  < imm ? 1:0;            break;
  case SLTIU: rd = rs1  < imm ? 1:0;            break;
  case XORI:  rd = rs1  ^ imm;                  break;
  case ORI:   rd = rs1  | imm;                  break;
  case ANDI:  rd = rs1  & imm;                  break;
  case SLLI:  rd = rs1 << imm;                  break;
  case SRLI:  rd = rs1 >> imm;                  break;
  case SRAI:  rd = rs1 >> imm;                  break;
  case ADD:   rd = rs1  + rs2;                  break;
  case SUB:   rd = rs1  - rs2;                  break;
  case SLL:   rd = rs1 << slice(rs2, 0, 4);     break;
  case SLT:   rd = (int) rs1 < (int) rs2 ? 1:0; break;
  case SLTU:  rd = rs1  < rs2 ? 1:0;            break;
  case XOR:   rd = rs1  ^ rs2;                  break;
  case SRL:   rd = rs1 >> slice(rs2, 0, 4);     break;
  case SRA:   rd = rs1 >> slice(rs2, 0, 4);     break;
  case OR:    rd = rs1  | rs2;                  break;
  case AND:   rd = rs1  & rs2;                  break;
  case FENCE:                                   break;
  case FENCE_TSO:                               break;
  case PAUSE:                                   break;
  case ECALL:                                   break;
  case EBREAK:                                  break;
  case RVVM_ERR: print("rvvm_err at ", Bits<64>(pc));break;
  default:                                      break;
  }
  pc += 4;
}

} // namespace rvvm
