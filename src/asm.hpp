struct Instruction_t {
  string          name;
  Opcode          opcode;
  InstructionType type;
  void exec(Machine& machine)
};

map<Opcode, string> opcode_str {
  { Opcode::LUI        , "lui"         },
  { Opcode::AUIPC      , "auipc"       },
  { Opcode::JAL        , "jal"         },
  { Opcode::JALR       , "jalr"        },
  { Opcode::BRANCH     , "branch"      },
  { Opcode::LOAD       , "load"        },
  { Opcode::STORE      , "store"       },
  { Opcode::OP_IMM     , "op_imm"      },
  { Opcode::OP         , "op"          },
  { Opcode::MISC_MEM   , "misc_mem"    },
  { Opcode::SYSTEM     , "system"      },
  { Opcode::RVVM_NOIMPL, "rvvm_noimpl" },
};

map<Instruction, string> inst_str { 
  { Instruction::ADDI,      "addi"     },
  { Instruction::LUI,       "lui"      },
  { Instruction::AUIPC,     "auipc"    },
  { Instruction::JAL,       "jal"      },
  { Instruction::JALR,      "jalr"     },
  { Instruction::BEQ,       "beq"      },
  { Instruction::BNE,       "bne"      },
  { Instruction::BLT,       "blt"      },
  { Instruction::BGE,       "bge"      },
  { Instruction::BLTU,      "bltu"     },
  { Instruction::BGEU,      "bgeu"     },
  { Instruction::LB,        "lb"       },
  { Instruction::LH,        "lh"       },
  { Instruction::LW,        "lw"       },
  { Instruction::LBU,       "lbu"      },
  { Instruction::LHU,       "lhu"      },
  { Instruction::SB,        "sb"       },
  { Instruction::SH,        "sh"       },
  { Instruction::SW,        "sw"       },
  { Instruction::ADDI,      "addi"     },
  { Instruction::SLTI,      "slti"     },
  { Instruction::SLTIU,     "sltiu"    },
  { Instruction::XORI,      "xori"     },
  { Instruction::ORI,       "ori"      },
  { Instruction::ANDI,      "andi"     },
  { Instruction::SLLI,      "slli"     },
  { Instruction::SRLI,      "srli"     },
  { Instruction::SRAI,      "srai"     },
  { Instruction::ADD,       "add"      },
  { Instruction::SUB,       "sub"      },
  { Instruction::SLL,       "sll"      },
  { Instruction::SLT,       "slt"      },
  { Instruction::SLTU,      "sltu"     },
  { Instruction::XOR,       "xor"      },
  { Instruction::SRL,       "srl"      },
  { Instruction::SRA,       "sra"      },
  { Instruction::OR,        "or"       },
  { Instruction::AND,       "and"      },
  { Instruction::FENCE,     "fence"    },
  { Instruction::FENCE_TSO, "fence_tso"},
  { Instruction::PAUSE,     "pause"    },
  { Instruction::ECALL,     "ecall"    },
  { Instruction::EBREAK,    "ebreak"   },
  { Instruction::RVVM_ERR,  "rvvm_err" },
};

map<u8, string> regname {
  {  0, "x0"  },
  {  1, "x1"  },
  {  2, "x2"  },
  {  3, "x3"  },
  {  4, "x4"  },
  {  5, "x5"  },
  {  6, "x6"  },
  {  7, "x7"  },
  {  8, "x8"  },
  {  9, "x9"  },
  { 10, "x10" },
  { 11, "x11" },
  { 12, "x12" },
  { 13, "x13" },
  { 14, "x14" },
  { 15, "x15" },
  { 16, "x16" },
  { 17, "x17" },
  { 18, "x18" },
  { 19, "x19" },
  { 20, "x20" },
  { 21, "x21" },
  { 22, "x22" },
  { 23, "x23" },
  { 24, "x24" },
  { 25, "x25" },
  { 26, "x26" },
  { 27, "x27" },
  { 28, "x28" },
  { 29, "x29" },
  { 30, "x30" },
  { 31, "x31" },
};

map<u8, string> regname_conv {
  {  0, "x0"  },
  {  1, "x1"  },
  {  2, "x2"  },
  {  3, "x3"  },
  {  4, "x4"  },
  {  5, "x5"  },
  {  6, "x6"  },
  {  7, "x7"  },
  {  8, "x8"  },
  {  9, "x9"  },
  { 10, "x10" },
  { 11, "x11" },
  { 12, "x12" },
  { 13, "x13" },
  { 14, "x14" },
  { 15, "x15" },
  { 16, "x16" },
  { 17, "x17" },
  { 18, "x18" },
  { 19, "x19" },
  { 20, "x20" },
  { 21, "x21" },
  { 22, "x22" },
  { 23, "x23" },
  { 24, "x24" },
  { 25, "x25" },
  { 26, "x26" },
  { 27, "x27" },
  { 28, "x28" },
  { 29, "x29" },
  { 30, "x30" },
  { 31, "x31" },
};

struct Register {
  string name;
  string name_conv;
  u32 val;
};

namespace Registers {

// regname, regname_conv
Register x0  {  "x0", "zero" };
Register x1  {  "x1", "ra"   };
Register x2  {  "x2", "sp"   };
Register x3  {  "x3", "gp"   };
Register x4  {  "x4", "tp"   };
Register x5  {  "x5", "t0"   };
Register x6  {  "x6", "t1"   };
Register x7  {  "x7", "t2"   };
Register x8  {  "x8", "s0"   };
Register x9  {  "x9", "s1"   };
Register x10 { "x10", "a0"   };
Register x11 { "x11", "a1"   };
Register x12 { "x12", "a2"   };
Register x13 { "x13", "a3"   };
Register x14 { "x14", "a4"   };
Register x15 { "x15", "a5"   };
Register x16 { "x16", "a6"   };
Register x17 { "x17", "a7"   };
Register x18 { "x18", "x18"  };
Register x19 { "x19", "s3"   };
Register x20 { "x20", "s4"   };
Register x21 { "x21", "s5"   };
Register x22 { "x22", "s6"   };
Register x23 { "x23", "s7"   };
Register x24 { "x24", "s8"   };
Register x25 { "x25", "s9"   };
Register x26 { "x26", "s10"  };
Register x27 { "x27", "s11"  };
Register x28 { "x28", "t3"   };
Register x29 { "x29", "t4"   };
Register x30 { "x30", "t5"   };
Register x31 { "x31", "t6"   };

}

map<u8, Register> registers;

// print instruction name, operands
string disassemble(u32 i) {
  stringstream ss;
  ss << inst_str[get_instruction(i)];
  switch (get_type(i)) {
  using enum InstructionType;
  case R: // rd = rs1 op rs2
    ss << " " << regname[get_rd(i)]
       << " " << regname[get_rs1(i)]
       << " " << regname[get_rs2(i)];
    break;
  case I: // rd = rs1 op imm
    ss << " " << regname[get_rd(i)]
       << " " << regname[get_rs1(i)]
       << " " << hex_(get_imm(i));
    break;
  case S:
    ss << " " << regname[get_rd(i)]
       << " " << regname[get_rs1(i)]
       << " " << regname[get_rs2(i)];
    break;
  case B: // rd = rs1 op offs
    ss << " " << regname[get_rd(i)]
       << " " << regname[get_rs1(i)]
       << " " << regname[get_rs2(i)];
    break;
  case U:
    ss << " " << regname[get_rd(i)]
       << " " << regname[get_rs1(i)]
       << " " << regname[get_rs2(i)];
    break;
  case J:
    ss << " " << regname[get_rd(i)]
       << " " << regname[get_rs1(i)]
       << " " << regname[get_rs2(i)];
    break;
  }
  return ss.str();
}
