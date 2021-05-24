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
using byte = u8;
using word = u32;

////////////////////////////////////////////////////////////
// bit manipulation and retrieval
////////////////////////////////////////////////////////////
inline auto slice(word w, int l, int u) {
  return (word) (w << (31 - u)) >> (31 - u + l);
}

// returns word with bits w[u:l] ones and zeroes else
inline auto ones(int l, int u) {
  return ((word) slice(-1, l, u)) << l;
}

inline auto zeroes(int l, int u) {
  return ~ones(l, u);
}

inline auto slice_mask(int l, int u) {
  /* return (((word) -1) << l) >> (31 - u); */
  return ((word) slice(-1, l, u)) << l;
}

// get bits w[u:l] as least significant digits
inline word get_slice(word w, int l, int u) {
  return (word) (w << (31 - u)) >> (31 - u + l);
}

// changes bits w[u:l] to the bits v[u-l+1:0]
// soon: w.at(l, u) = v.at(0, u - l);
// align v with slice and zero bits not in slice
// zero bits in slice and set them to the slice in v
inline void set_slice(word& w, word v, int l, int u) {
  w = (w & ~slice_mask(l, u))
    | ((v << l) & slice_mask(l, u));
}

inline String bits(byte w) {
  String out = "0b";
  for (auto i : range(1, 0)) // 2 apostroph-seperated nibbles
    { out += "'"; out += std::bitset<4>(get_slice(w, i * 4, (i * 4) + 3)).to_string(); }
  return out;
}

inline String bits(word w) {
  String out = "0b";
  for (auto i : range(7, 0)) // print 8 apostroph-seperated nibbles
    { out += "'"; out += std::bitset<4>(get_slice(w, i * 4, (i * 4) + 3)).to_string(); }
  return out;
  /* put("0b'", std::bitset<4>(get_slice(w, 4, 7)), */
  /*       "'", std::bitset<4>(get_slice(w, 0, 3))); */
}

inline auto print_bits(byte w) { put(bits(w)); }

inline auto hex(word w) { put("0x'", std::hex, w); }

inline auto print_hex(word w) { print("0x'", std::hex, w); }

inline auto print_bits(word w) {
  put("0b");
  for (auto i : range(7, 0)) // print 8 apostroph-seperated nibbles
    put("'", std::bitset<4>(get_slice(w, i * 4, (i * 4) + 3)));
  print();
}

inline auto set_bits(word& w, int l, int u) { w |= slice_mask(l, u); }

inline auto bit_at(word w, int i) { return get_slice(w, i, i); }

// sign extend
inline auto sxt(word& w, int from, int to) {
  if (bit_at(w, from)) w |=  slice_mask(from + 1, to);
  else                 w &= ~slice_mask(from + 1, to);
}

inline auto sxt(word& w, int from) { sxt(w, from, 31); }

// Word structure for processing all machine words
/* struct Word : public word { // alternative */
struct Word {
  word w;

  Word(word _w) : w{_w} {}
  Word()        : Word(0)  {}
  
  auto at(u32 i)                  { return get_slice(w, i, i); }
  auto at(u32 l, u32 u)           { return get_slice(w, l, u); }

  auto operator()(u32 i)          { return at(i, i); }
  auto operator()(u32 l, u32 u)   { return at(l, u); }

  auto set(word v, word l, word u) { return set_slice(w, v, l, u); }
  auto set(word v)                 { return set_slice(w, v, 0, 31); }

  String binary() { }
  String hex() { }
};



////////////////////////////////////////////////////////////
// rvvm
////////////////////////////////////////////////////////////

namespace rvvm {

////////////////////////////////////////////////////////////
// RISC-V Opcodes, Instruction Types, Instructions
////////////////////////////////////////////////////////////

enum class Opcode {
  LUI      = 0b011'0111,
  AUIPC    = 0b001'0111,
  JAL      = 0b110'1111,
  JALR     = 0b110'0111,
  BRANCH   = 0b110'0011,
  LOAD     = 0b000'0011,
  STORE    = 0b010'0011,
  IMM      = 0b001'0011,
  OP       = 0b011'0011,
  MISC_MEM = 0b000'1111,
  SYSTEM   = 0b111'0011,
  NOIMPL,
};

enum class InstructionType { R, I, S, B, U, J, };

enum class Operation {
  LUI, AUIPC, JAL, JALR, BEQ, BNE, BLT, BGE, BLTU, BGEU,
  LB, LH, LW, LBU, LHU, SB, SH, SW, ADDI, SLTI, SLTIU,
  XORI, ORI, ANDI, SLLI, SRLI, SRAI, ADD, SUB, SLL, SLT, SLTU,
  XOR, SRL, SRA, OR, AND, FENCE, FENCE_TSO, PAUSE,
  ECALL, EBREAK, DECODE_ERR, NOIMPL,
};



////////////////////////////////////////////////////////////
// Instruction object encapsulates all decoding and encoding
////////////////////////////////////////////////////////////

struct Instruction : Word {
  word            w;
  Opcode          opcode;
  word            rd;
  word            rs1;
  word            rs2;
  word            funct3;
  word            funct7;
  Operation       operation;
  InstructionType type;
  word            imm;

  Instruction(word _w) { // establish invariant instruction fields
    w         = _w;
    opcode    = Opcode(Word(w)(0, 6));
    rd        = Word(w)( 7, 11);
    rs1       = Word(w)(15, 19);
    rs2       = Word(w)(20, 24);
    funct3    = Word(w)(12, 14);
    funct7    = Word(w)(25, 31);
    switch (opcode) { // get type
    case Opcode::LUI      : type = InstructionType::U; break;
    case Opcode::AUIPC    : type = InstructionType::U; break;
    case Opcode::JAL      : type = InstructionType::J; break;
    case Opcode::JALR     : type = InstructionType::I; break;
    case Opcode::BRANCH   : type = InstructionType::B; break;
    case Opcode::LOAD     : type = InstructionType::I; break;
    case Opcode::STORE    : type = InstructionType::S; break;
    case Opcode::IMM      : type = InstructionType::I; break;
    case Opcode::OP       : type = InstructionType::R; break;
    case Opcode::MISC_MEM : type = InstructionType::I; break;
    case Opcode::SYSTEM   : type = InstructionType::I; break;
    case Opcode::NOIMPL   : type = InstructionType::I; break;
    default               : type = InstructionType::I; break;
    }
    switch (type) { // get immediate
    case InstructionType::I : imm = get_slice(w, 20, 31);                    break;
    case InstructionType::S : imm = (Word(w)(25, 31) << 5) | Word(w)(7, 11); break;
    case InstructionType::B : imm = word(0);                                 break;
    case InstructionType::U : imm = get_slice(w, 12, 31) << 12;              break;
    case InstructionType::J : imm = word(0);                                 break;
    default                 : imm = word(0);                                 break;
    }
    switch (opcode) { // get operation (instruction-decoding)
    case Opcode::LUI      : operation = Operation::LUI;                             break;
    case Opcode::AUIPC    : operation = Operation::AUIPC;                           break;
    case Opcode::JAL      : operation = Operation::JAL;                             break;
    case Opcode::JALR     : operation = Operation::JALR;                            break;
    case Opcode::BRANCH   : switch (funct3) {
      case 0b000          : operation = Operation::BEQ;                             break;
      case 0b001          : operation = Operation::BNE;                             break;
      case 0b100          : operation = Operation::BLT;                             break;
      case 0b101          : operation = Operation::BGE;                             break;
      case 0b110          : operation = Operation::BLTU;                            break;
      case 0b111          : operation = Operation::BGEU;                            break;
      default             : operation = Operation::DECODE_ERR;                      break;
      } break;
    case Opcode::LOAD     : switch (funct3) {
      case 0b000          : operation = Operation::LB;                              break;
      case 0b001          : operation = Operation::LH;                              break;
      case 0b010          : operation = Operation::LW;                              break;
      case 0b100          : operation = Operation::LBU;                             break;
      case 0b101          : operation = Operation::LHU;                             break;
      default             : operation = Operation::DECODE_ERR;                      break;
      } break;
    case Opcode::STORE    : switch (funct3) {
      case 0b000          : operation = Operation::SB;                              break;
      case 0b001          : operation = Operation::SH;                              break;
      case 0b010          : operation = Operation::SW;                              break;
      default             : operation = Operation::DECODE_ERR;                      break;
      } break;
    case Opcode::IMM      : switch (funct3) {
      case 0b000          : operation = Operation::ADDI;                            break;
      case 0b010          : operation = Operation::SLTI;                            break;
      case 0b011          : operation = Operation::SLTIU;                           break;
      case 0b100          : operation = Operation::XORI;                            break;
      case 0b110          : operation = Operation::ORI;                             break;
      case 0b111          : operation = Operation::ANDI;                            break;
      case 0b001          : operation = Operation::SLLI;                            break;
      case 0b101          : operation = funct7 ? Operation::SRAI : Operation::SRLI; break;
      default             : operation = Operation::DECODE_ERR;                      break;
      } break;
    case Opcode::OP       : switch (funct3) {
      case 0b000          : operation = funct7 ? Operation::SUB : Operation::ADD;   break;
      case 0b001          : operation = Operation::SLL;                             break;
      case 0b010          : operation = Operation::SLT;                             break;
      case 0b011          : operation = Operation::SLTU;                            break;
      case 0b100          : operation = Operation::XOR;                             break;
      case 0b101          : operation = funct7 ? Operation::SRA : Operation::SRL;   break;
      case 0b110          : operation = Operation::OR;                              break;
      case 0b111          : operation = Operation::AND;                             break;
      default             : operation = Operation::DECODE_ERR;                      break;
      } break;
    case Opcode::MISC_MEM : operation = Operation::NOIMPL;                          break;
    case Opcode::SYSTEM   : operation = imm ? Operation::EBREAK : Operation::ECALL; break;
    default               : operation = Operation::DECODE_ERR;                      break;
    }
  }
};



////////////////////////////////////////////////////////////
// Registers
////////////////////////////////////////////////////////////

struct Registers {
  Vector<word> x = Vector<word>(32);
  word& operator[](word i) {
    if (i >= 32) olib::die("Bad Register Index: ", i);
    return i ? x[i] : word();
  }
  void print(word count) { // print first count registers
    count = min(count, 32);
    olib::print("==================== registers> ====================");
    for (auto i : range_(count))
      olib::print("x", i, "\t= ", bits(operator[](i)));
    olib::print("==================== <registers ====================");
  }
};



////////////////////////////////////////////////////////////
// Memory
////////////////////////////////////////////////////////////

struct Memory {
  Map<word, byte> mem; // map for random access without huge contiguous array

  byte get_byte (word i) {
    return mem[i];
  }
  word get_hword(word i) {
    return (mem[i + 0] << (0 * 8))
        &  (mem[i + 1] << (1 * 8));
  }
  word get_word (word i) { // get little endian 4 byte word
    return (mem[i + 0] << (0 * 8))
        &  (mem[i + 1] << (1 * 8))
        &  (mem[i + 2] << (2 * 8))
        &  (mem[i + 3] << (3 * 8));
  }

  void set_byte (word i, word value) {
    mem[i] = get_slice(0, 7);
  }
  void set_hword(word i, word value) {
    mem[i + 0] = get_slice(value,  0,  7);
    mem[i + 1] = get_slice(value,  8, 15);
  }
  void set_word (word i, word value) {
    mem[i + 0] = get_slice(value,  0,  7);
    mem[i + 1] = get_slice(value,  8, 15);
    mem[i + 2] = get_slice(value, 16, 23);
    mem[i + 3] = get_slice(value, 24, 31);
  }

  void print() {
    olib::print("==================== memory>    ====================");
    for (auto [key, value] : mem)
      olib::print("mem[", key, "]\t= ", bits(value));
    olib::print("==================== <memory    ====================\n");
  }
};



////////////////////////////////////////////////////////////
// Cpu
////////////////////////////////////////////////////////////

struct Cpu {
  Registers    x;
  Memory       mem;
  word         pc;
  Instruction  instruction;
  Instruction  instruction(Instruction i) { return (instruction = i); }
  Instruction  instruction()              { return instruction(mem.get_word(pc)); }

  void exec(Instruction instruction) {
    auto& rd        = x[instruction.rd ];
    auto& rs1       = x[instruction.rs1];
    auto& rs2       = x[instruction.rs2];
    auto  imm       =   instruction.imm;
    switch             (instruction.operation) {
    case Operation::LUI        : rd = imm;                              break;
    case Operation::AUIPC      : pc += imm;                             break;
    case Operation::JAL        : rd = pc + imm; pc += imm;              break;
    case Operation::JALR       : rd = rs1 + imm;                        break;
    case Operation::BEQ        : rs1 == rs2 ? pc += imm :;              break; 
    case Operation::BNE        : rs1 != rs2 ? pc += imm :;              break;
    case Operation::BLT        : rs1  < rs2 ? pc += imm :;              break;
    case Operation::BGE        : rs1 >= rs2 ? pc += imm :;              break;
    case Operation::BLTU       : rs1  < rs2 ? pc += imm :;              break;
    case Operation::BGEU       : rs1 >= rs2 ? pc += imm :;              break;
    case Operation::LB         : rd = mem.get_byte  (rs1 + imm);        break;
    case Operation::LH         : rd = mem.get_hword (rs1 + imm);        break;
    case Operation::LW         : rd = mem.get_word  (rs1 + imm);        break;
    case Operation::LBU        : rd = mem.get_byte  (rs1 + imm);        break;
    case Operation::LHU        : rd = mem.get_hword (rs1 + imm);        break;
    case Operation::SB         : mem.set_byte  (rs1 + imm, rs2(0,  7)); break;
    case Operation::SH         : mem.set_hword (rs1 + imm, rs2(0, 15)); break;
    case Operation::SW         : mem.set_word  (rs1 + imm, rs2       ); break;
    case Operation::ADDI       : rd = rs1  + imm;                       break;
    case Operation::SLTI       : rd = rs1  < imm ? 1:0;                 break;
    case Operation::SLTIU      : rd = rs1  < imm ? 1:0;                 break;
    case Operation::XORI       : rd = rs1  ^ imm;                       break;
    case Operation::ORI        : rd = rs1  | imm;                       break;
    case Operation::ANDI       : rd = rs1  & imm;                       break;
    case Operation::SLLI       : rd = rs1 << imm;                       break;
    case Operation::SRLI       : rd = rs1 >> imm;                       break;
    case Operation::SRAI       : rd = rs1 >> imm;                       break;
    case Operation::ADD        : rd = rs1  + rs2;                       break;
    case Operation::SUB        : rd = rs1  - rs2;                       break;
    case Operation::SLL        : rd = rs1 << rs2(0, 4);                 break;
    case Operation::SLT        : rd = (int) rs1 < (int) rs2 ? 1:0;      break;
    case Operation::SLTU       : rd = rs1  < rs2 ? 1:0;                 break;
    case Operation::XOR        : rd = rs1  ^ rs2;                       break;
    case Operation::SRL        : rd = rs1 >> rs2(0, 4);                 break;
    case Operation::SRA        : rd = rs1 >> rs2(0, 4);                 break;
    case Operation::OR         : rd = rs1  | rs2;                       break;
    case Operation::AND        : rd = rs1  & rs2;                       break;
    case Operation::FENCE      :                                        break;
    case Operation::FENCE_TSO  :                                        break;
    case Operation::PAUSE      :                                        break;
    case Operation::ECALL      :                                        break;
    case Operation::EBREAK     :                                        break;
    case Operation::DECODE_ERR :                                        break;
    case Operation::NOIMPL     :                                        break;
    default                    :                                        break;
    }
    pc += 4;
  }
};

                    
                    

////////////////////////////////////////////////////////////
// Program
////////////////////////////////////////////////////////////

struct Program {
  auto instructions = Vector<Instruction>();
  auto data_segment = Vector<byte>();

  auto add_instr(Instruction w) { instructions.push_back(w); }
  auto add_data(byte w)         { data_segment.push_back(w); }

  Program(String filename) {
    std::ifstream file;
    file.open(filename, std::ios::in);

    if (!file) die("Failed to open file ", filename);

    auto lines = Vector<String>();

    for (auto line : lines)
      add_instr(Instruction(line));
  }

  Program(char **filename_chars) {
    auto filename = (String) filename_chars[1];
    Program(filename);
  }
};



////////////////////////////////////////////////////////////
// Assembler
////////////////////////////////////////////////////////////

auto nextToken() { } 

Map<String, Operation> operation_from_string {
  { "lui"       , Operation::LUI       },
  { "auipc"     , Operation::AUIPC     },
  { "jal"       , Operation::JAL       },
  { "jalr"      , Operation::JALR      },
  { "beq"       , Operation::BEQ       },
  { "bne"   	  , Operation::BNE       },
  { "blt"   	  , Operation::BLT       },
  { "bge"   	  , Operation::BGE       },
  { "bltu"      , Operation::BLTU      },
  { "bgeu"      , Operation::BGEU      },
  { "lb"	      , Operation::LB        },
  { "lh"	      , Operation::LH        },
  { "lw"	      , Operation::LW        },
  { "lbu"	      , Operation::LBU       },
  { "lhu"	      , Operation::LHU       },
  { "sb"	      , Operation::SB        },
  { "sh"	      , Operation::SH        },
  { "sw"	      , Operation::SW        },
  { "addi"	    , Operation::ADDI      },
  { "slti"	    , Operation::SLTI      },
  { "sltiu"	    , Operation::SLTIU     },
  { "xori"	    , Operation::XORI      },
  { "ori"	      , Operation::ORI       },
  { "andi"	    , Operation::ANDI      },
  { "slli"	    , Operation::SLLI      },
  { "srli"	    , Operation::SRLI      },
  { "srai"	    , Operation::SRAI      },
  { "add"	      , Operation::ADD       },
  { "sub"	      , Operation::SUB       },
  { "sll"	      , Operation::SLL       },
  { "slt"	      , Operation::SLT       },
  { "sltu"      , Operation::SLTU      },
  { "xor"       , Operation::XOR       },
  { "srl"	      , Operation::SRL       },
  { "sra"	      , Operation::SRA       },
  { "or"	      , Operation::OR        },
  { "and"	      , Operation::AND       },
  { "fence"     , Operation::FENCE     },
  { "fence_tso" , Operation::FENCE_TSO },
  { "pause"     , Operation::PAUSE     },
  { "ecall"     , Operation::ECALL     },
  { "ebreak"    , Operation::EBREAK    },
};

auto assemble_line(String line) { }

auto assemble_program(Vector<String> lines) { for (auto line : lines) break; }



////////////////////////////////////////////////////////////
// Interpreter
////////////////////////////////////////////////////////////

constexpr String PROMPT = "rvvm>";

void rvvm_run_files(int argc, char **filenames) {
  auto files   = get_args(argc, filenames);
  auto program = Program(files[1]);
  auto cpu     = Cpu();
  if (argc > 2)
    cpu.mem = Memory(files[2]);
  for (auto instruction : program.instructions);
    cpu.exec(instruction);
}

void rvvm_run_shell() {
  for (String line; line = getline(PROMPT);)
    cpu.exec(Instruction(line));
}

}; // namespace rvvm
