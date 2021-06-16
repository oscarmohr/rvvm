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

#include "olib.hpp"   // Own Library (for its development)
using namespace olib; // String, getline, u8, ...

namespace rvvm {

////////////////////////////////////////////////////////////
// Bit Manipulation
////////////////////////////////////////////////////////////

using word = u32; // see olib for u8, u32 fixed width int

word get_slice(word w, word l, word u) { // get bits w[u:l] as lsd's
  word shamt_l = 31 - u;
  word shamt_r = 31 - u + l;
  return  (w << shamt_l) >> shamt_r;
}
word ones  (word l, word u) { return get_slice(-1, l, u) << l; } 
word zeroes(word l, word u) { return ~ones(l, u);              }

void set_slice(word& w, word v, word l, word u) {
  w = (w & zeroes(l, u))  |  ((v << l) & ones(l, u));
}


////////////////////////////////////////////////////////////
// Abstract Machine Word
////////////////////////////////////////////////////////////

struct Word { // abstract machine word
  word w;

  Word(word _w) : w{_w}      {}
  Word()        : Word(0)    {}

  Word operator=  (word rhs) { return w = rhs;          } // copy assignment
  Word operator=  (Word rhs) { return w = rhs.w;        }
  bool operator== (word rhs) { return w == rhs;         } // comparison
  bool operator== (Word rhs) { return w == rhs.w;       }
  bool operator<= (word rhs) { return w <= rhs;         }
  bool operator<= (Word rhs) { return w <= rhs.w;       }
  bool operator<  (word rhs) { return w <  rhs;         }
  bool operator<  (Word rhs) { return w <  rhs.w;       }
  bool operator>= (word rhs) { return w >= rhs;         }
  bool operator>= (Word rhs) { return w >= rhs.w;       }
  bool operator>  (word rhs) { return w >  rhs;         }
  bool operator>  (Word rhs) { return w >  rhs.w;       }
  Word operator+  (word rhs) { return Word(w  + rhs  ); } // arithmetic
  Word operator+  (Word rhs) { return Word(w  + rhs.w); }
  Word operator-  (word rhs) { return Word(w  - rhs  ); }
  Word operator-  (Word rhs) { return Word(w  - rhs.w); }

  Word operator<< (word rhs) { return Word(w << rhs  ); } // bitshifts
  Word operator<< (Word rhs) { return Word(w << rhs.w); }
  Word operator>> (word rhs) { return Word(w >> rhs  ); }
  Word operator>> (Word rhs) { return Word(w >> rhs.w); }

  Word operator&  (word rhs) { return Word(w  & rhs  ); } // logic
  Word operator&  (Word rhs) { return Word(w  & rhs.w); }
  Word operator|  (word rhs) { return Word(w  | rhs  ); }
  Word operator|  (Word rhs) { return Word(w  | rhs.w); }
  Word operator^  (word rhs) { return Word(w  ^ rhs  ); }
  Word operator^  (Word rhs) { return Word(w  ^ rhs.w); }
  Word operator~  ()         { return Word(~w);         }

  Word get(u32 l, u32 u)           { return Word(get_slice(w, l, u)); }
  Word get(u32 i)                  { return Word(get_slice(w, i, i)); }
  void set(Word v, word l, word u) { set_slice(w, v.w, l, u); }
  void set(word v)                 { set(v, 0, 31); }
  Word operator()(word l, word u)  { return get(l, u); } // for w(0, 31)
  Word operator()(word i)          { return get(i, i); }

  void sxt(word from, word to) { // sign extend
    w = get_slice(w, from, from)
        ? w |  ones(from + 1, to)
        : w & ~ones(from + 1, to);
  }
  void sxt(word from) { sxt(from, 31); } // might add RV64 support

  ////////////////////////////////////////////////////////////
  String binary() {
    String out = "0b";
    for (auto i : range(7, 0)) // 8 apostroph-seperated nibbles
      out += "'" + Bits<4>(get_slice(w, i*4, (i*4) + 3)).to_string();
    return out;
  }
  void put() { olib::put(binary()); }
};



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

Opcode opcode_of(Word w) { return Opcode(0b0110111); }

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

struct Instruction : Word { // execution context of machine word
  Opcode          opcode;
  word            rd;
  word            rs1;
  word            rs2;
  word            funct3;
  word            funct7;
  Operation       operation;
  InstructionType type;
  word            imm;

  Instruction(word _w) : Word(_w) { // establish invariant instruction fields
    opcode    = Opcode(Word(w)(0, 6).w);
    rd        = Word(w)( 7, 11).w;
    rs1       = Word(w)(15, 19).w;
    rs2       = Word(w)(20, 24).w;
    funct3    = Word(w)(12, 14).w;
    funct7    = Word(w)(25, 31).w;

    switch (opcode) { // decoding: get type
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
  Vector<Word> x = Vector<Word>(33); // 32 regs + 1 dummy reg

  Word& operator[](word i) {
    if (i >= 32) olib::die("Bad Register Index: ", i);
    return i ? x[i] : (x[32] = 0); // return dummy reg x32=0 instead of x0
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
  Map<Word, Word> mem; // space-efficient, simple random access

  Word& operator[](Word i) { return mem[i.w]       }
  Word& get(Word i)        { return operator[](i); }

  Word get_byte (Word i) { // sign extend
    return get(i).sxt(8, 31);
  }
  Word get_hword(Word i) { // sign extend
    return (get(i + 0) << (0 * 8))
        &  (get(i + 1) << (1 * 8));
  }
  Word get_word (Word i) { // get little endian 4 byte word
    return (get(i + 0) << (0 * 8))
        &  (get(i + 1) << (1 * 8))
        &  (get(i + 2) << (2 * 8))
        &  (get(i + 3) << (3 * 8));
  }

  void set_byte (Word i, word value) {
    get(i) = get_slice(i, 0, 7); // bits w[:7] are 0
  }
  void set_hword(Word i, word value) {
    get(i + 0) = get_slice(value,  0,  7);
    get(i + 1) = get_slice(value,  8, 15);
  }
  void set_word (Word i, word value) {
    get(i + 0) = get_slice(value,  0,  7);
    get(i + 1) = get_slice(value,  8, 15);
    get(i + 2) = get_slice(value, 16, 23);
    get(i + 3) = get_slice(value, 24, 31);
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

  void exec(Instruction instruction) {
    Word& rd        = x[instruction.rd ];
    Word& rs1       = x[instruction.rs1];
    Word& rs2       = x[instruction.rs2];
    Word  imm       =   instruction.imm;
    switch             (instruction.operation) {
    case Operation::LUI        : rd = imm;                              break;
    case Operation::AUIPC      : pc += imm;                             break;
    case Operation::JAL        : rd = pc + imm; pc += imm;              break;
    case Operation::JALR       : rd = rs1 + imm;                        break;
    case Operation::BEQ        : pc += rs1 == rs2 ? imm : 0;            break; 
    case Operation::BNE        : pc += rs1 != rs2 ? imm : 0;            break;
    case Operation::BLT        : pc += rs1  < rs2 ? imm : 0;            break;
    case Operation::BGE        : pc += rs1 >= rs2 ? imm : 0;            break;
    case Operation::BLTU       : pc += rs1  < rs2 ? imm : 0;            break;
    case Operation::BGEU       : pc += rs1 >= rs2 ? imm : 0;            break;
    case Operation::LB         : rd = mem.get_byte  (rs1 + imm);        break;
    case Operation::LH         : rd = mem.get_hword (rs1 + imm);        break;
    case Operation::LW         : rd = mem.get_word  (rs1 + imm);        break;
    case Operation::LBU        : rd = mem.get_byte  (rs1 + imm);        break;
    case Operation::LHU        : rd = mem.get_hword (rs1 + imm);        break;
    case Operation::SB         : mem.set_byte  (rs1 + imm, get_slice(rs2, 0,  7)); break;
    case Operation::SH         : mem.set_hword (rs1 + imm, get_slice(rs2, 0, 15)); break;
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
    case Operation::SLL        : rd = rs1 << get_slice(rs2, 0, 4);                 break;
    case Operation::SLT        : rd = (int) rs1 < (int) rs2 ? 1:0;      break;
    case Operation::SLTU       : rd = rs1  < rs2 ? 1:0;                 break;
    case Operation::XOR        : rd = rs1  ^ rs2;                       break;
    case Operation::SRL        : rd = rs1 >> get_slice(rs2, 0, 4);                 break;
    case Operation::SRA        : rd = rs1 >> get_slice(rs2, 0, 4);                 break;
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
  Vector<Instruction> instructions = Vector<Instruction>();
  Vector<byte>        data_segment = Vector<byte>();

  auto add_instr(Instruction w) { instructions.push_back(Instruction(w)); }
  auto add_data(byte w)         { data_segment.push_back(w); }

  Program(String filename) {
    std::ifstream file;
    file.open(filename, std::ios::in);

    if (!file) die("Failed to open file ", filename);

    auto lines = Vector<String>();

    /* for (auto line : lines) add_instr(Instruction(line)); */
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

String PROMPT = "rvvm>";

/* void rvvm_run_files(int argc, char **filenames) { */
/*   auto files   = get_args(argc, filenames); */
/*   auto program = Program(files[1]); */
/*   auto cpu     = Cpu(); */
/*   if (argc > 2) */
/*     cpu.mem = Memory(files[2]); */
/*   for (auto instruction : program.instructions); */
/*     cpu.exec(instruction); */
/* } */

/* void rvvm_run_shell() { */
/*   for (String line; line = getline(PROMPT);) */
/*     cpu.exec(Instruction(line)); */
/* } */


