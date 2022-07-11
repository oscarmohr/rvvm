#include <algorithm>
#include <functional>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <filesystem>
#include <cstdlib>

using i32 = int32_t;
using i16 = int16_t;
using i8  = int8_t;
using u32 = uint32_t;
using u16 = uint16_t;
using u8  = uint8_t;

auto log(const auto&... args) {
  (std::cerr << ... << args) << '\n';
}

auto put(const auto&... args) {
  (void)(std::cout << ... << args);
}

auto print(const auto&... args) {
  put(args..., '\n');
}

auto str(const auto&... args) {
  return (std::stringstream() << ... << args).str();
}

auto byte_to_hex(auto x) {
  return str(std::hex, std::setw(2), std::setfill('0'), u32(x));
}

auto to_hex(u32 x) {
  auto byte_0 = byte_to_hex(x & 0xff);
  auto byte_1 = byte_to_hex((x >>= 8) & 0xff);
  auto byte_2 = byte_to_hex((x >>= 8) & 0xff);
  auto byte_3 = byte_to_hex((x >>= 8) & 0xff);
  auto delim = '_';
  return str("0x", byte_3, delim, byte_2, delim, byte_1, delim, byte_0);
}

auto die(const auto&... args) {
  std::cout << std::flush;
  (std::cerr << ... << args) << '\n' << std::flush;
  std::exit(EXIT_FAILURE);
}

enum Instruction : size_t {
  LUI, AUIPC,
  JAL, JALR, BEQ, BNE, BLT, BGE, BLTU, BGEU,
  LB, LH, LW, LBU, LHU, SB, SH, SW,
  ADDI, SLTI, SLTIU, XORI, ORI, ANDI, SLLI, SRLI, SRAI,
  ADD, SUB, SLL, SLT, SLTU, XOR, SRL, SRA, OR, AND,
  EBREAK, UNDEF
};

enum OPCODE : u32 {
  OPCODE_LUI         = 0x37,
  OPCODE_AUIPC       = 0x17,
  OPCODE_JAL         = 0x6f,
  OPCODE_JALR        = 0x67,
  OPCODE_BRANCH      = 0x63,
  OPCODE_LOAD        = 0x03,
  OPCODE_STORE       = 0x23,
  OPCODE_OP_IMM      = 0x13,
  OPCODE_OP          = 0x33,
  OPCODE_SYSTEM      = 0x73,
};

enum FUNCT3 : u32 {
  FUNCT3_BEQ         = 0x0,
  FUNCT3_BNE         = 0x1,
  FUNCT3_BLT         = 0x4,
  FUNCT3_BGE         = 0x5,
  FUNCT3_BLTU        = 0x6,
  FUNCT3_BGEU        = 0x7,
  FUNCT3_LB          = 0x0,
  FUNCT3_LH          = 0x1,
  FUNCT3_LW          = 0x2,
  FUNCT3_LBU         = 0x4,
  FUNCT3_LHU         = 0x5,
  FUNCT3_SB          = 0x0,
  FUNCT3_SH          = 0x1,
  FUNCT3_SW          = 0x2,
  FUNCT3_ADDI        = 0x0,
  FUNCT3_SLTI        = 0x2,
  FUNCT3_SLTIU       = 0x3,
  FUNCT3_XORI        = 0x4,
  FUNCT3_ORI         = 0x6,
  FUNCT3_ANDI        = 0x7,
  FUNCT3_SLLI        = 0x1,
  FUNCT3_SRAI_SRLI   = 0x5,
  FUNCT3_SUB_ADD     = 0x0,
  FUNCT3_SLL         = 0x1,
  FUNCT3_SLT         = 0x2,
  FUNCT3_SLTU        = 0x3,
  FUNCT3_XOR         = 0x4,
  FUNCT3_SRA_SRL     = 0x5,
  FUNCT3_OR          = 0x6,
  FUNCT3_AND         = 0x7,
};

enum FUNCT7 : u32 {
  FUNCT7_SRAI        = 0x20,
  FUNCT7_SRLI        = 0,
  FUNCT7_SUB         = 0x20,
  FUNCT7_ADD         = 0,
  FUNCT7_SRA         = 0x20,
  FUNCT7_SRL         = 0,
};

// MASK_LO_HI[i] = 1 for i = LO, ..., HI else 0
enum MASK : u32 {
  MASK_00_06         = 0x0000007f,
  MASK_07_11         = 0x00000f80,
  MASK_15_19         = 0x000f8000,
  MASK_20_24         = 0x01f00000,
  MASK_12_14         = 0x00007000,
  MASK_25_31         = 0xfe000000,
  MASK_20_31         = 0xfff00000,
  MASK_08_11         = 0x00000f00,
  MASK_25_30         = 0x7e000000,
  MASK_07_07         = 0x00000080,
  MASK_31_31         = 0x80000000,
  MASK_12_31         = 0xfffff000,
  MASK_21_30         = 0x7fe00000,
  MASK_20_20         = 0x00100000,
  MASK_12_19         = 0x000ff000,
  MASK_OPCODE        = MASK_00_06,
  MASK_RD            = MASK_07_11,
  MASK_RS1           = MASK_15_19,
  MASK_RS2           = MASK_20_24,
  MASK_FUNCT3        = MASK_12_14,
  MASK_FUNCT7        = MASK_25_31,
  MASK_I_IMM_0       = MASK_20_31,
  MASK_S_IMM_0       = MASK_07_11,
  MASK_S_IMM_1       = MASK_25_31,
  MASK_B_IMM_0       = MASK_08_11,
  MASK_B_IMM_1       = MASK_25_30,
  MASK_B_IMM_2       = MASK_07_07,
  MASK_B_IMM_3       = MASK_31_31,
  MASK_U_IMM_0       = MASK_12_31,
  MASK_J_IMM_0       = MASK_21_30,
  MASK_J_IMM_1       = MASK_20_20,
  MASK_J_IMM_2       = MASK_12_19,
  MASK_J_IMM_3       = MASK_31_31,
};

enum OFFSET : u32 {
  OFFSET_OPCODE      = 0,
  OFFSET_RD          = 7,
  OFFSET_RS1         = 15,
  OFFSET_RS2         = 20,
  OFFSET_FUNCT3      = 12,
  OFFSET_FUNCT7      = 25,
  OFFSET_I_IMM_0     = 20,
  OFFSET_S_IMM_0     = 7,
  OFFSET_S_IMM_1     = 25,
  OFFSET_B_IMM_0     = 8,
  OFFSET_B_IMM_1     = 25,
  OFFSET_B_IMM_2     = 7,
  OFFSET_B_IMM_3     = 31,
  OFFSET_U_IMM_0     = 12,
  OFFSET_J_IMM_0     = 21,
  OFFSET_J_IMM_1     = 20,
  OFFSET_J_IMM_2     = 12,
  OFFSET_J_IMM_3     = 31,
};

// position of the immediate parts in their immediate
enum POS : u32 {
  POS_I_IMM_0        = 0,
  POS_S_IMM_0        = 0,
  POS_S_IMM_1        = 5,
  POS_B_IMM_0        = 1,
  POS_B_IMM_1        = 5,
  POS_B_IMM_2        = 11,
  POS_B_IMM_3        = 12,
  POS_U_IMM_0        = 12,
  POS_J_IMM_0        = 1,
  POS_J_IMM_1        = 11,
  POS_J_IMM_2        = 12,
  POS_J_IMM_3        = 20,
};

// sign bit position for sign extension
enum SXT_BIT : u32 {
  SXT_BIT_I_IMM = 11,
  SXT_BIT_S_IMM = 11,
  SXT_BIT_B_IMM = 12,
  SXT_BIT_U_IMM = 31,
  SXT_BIT_J_IMM = 20,
};

i32 sxt(i32 sxt_bit, i32 x) {
  auto shamt = 31 - sxt_bit; // shift off superflous bits
  return (x << shamt) >> shamt;
}

#define SLICE(M, X)        (((X) & MASK_##M) >> OFFSET_##M)
#define PART(IMM, PART, X) (SLICE(IMM##_IMM_##PART, X) << POS_##IMM##_IMM_##PART)

u32 get_opcode(u32 x) { return SLICE(OPCODE, x); }
u32 get_funct3(u32 x) { return SLICE(FUNCT3, x); }
u32 get_funct7(u32 x) { return SLICE(FUNCT7, x); }
u32 get_rd    (u32 x) { return SLICE(RD, x);     }
u32 get_rs1   (u32 x) { return SLICE(RS1, x);    }
u32 get_rs2   (u32 x) { return SLICE(RS2, x);    }

i32 get_I_imm(u32 x){
  auto imm = PART(I, 0, x);
  return sxt(SXT_BIT_I_IMM, imm);
}

i32 get_S_imm(u32 x){
  auto imm = PART(S, 0, x) | PART(S, 1, x);
  return sxt(SXT_BIT_S_IMM, imm);
}

i32 get_B_imm(u32 x){
  auto imm = PART(B, 0, x) | PART(B, 1, x) | PART(B, 2, x) | PART(B, 3, x);
  return sxt(SXT_BIT_B_IMM, imm);
}

i32 get_U_imm(u32 x){
  auto imm = PART(U, 0, x); 
  return sxt(SXT_BIT_U_IMM, imm);
}

i32 get_J_imm(u32 x){
  auto imm = PART(J, 0, x) | PART(J, 1, x) | PART(J, 2, x) | PART(J, 3, x);
  return sxt(SXT_BIT_J_IMM, imm);
}

i32 get_imm(u32 inst) {
  switch (get_opcode(inst)) {
  case OPCODE_LUI:    return get_U_imm(inst);
  case OPCODE_AUIPC:  return get_U_imm(inst);
  case OPCODE_JAL:    return get_J_imm(inst);
  case OPCODE_JALR:   return get_I_imm(inst);
  case OPCODE_BRANCH: return get_B_imm(inst);
  case OPCODE_LOAD:   return get_I_imm(inst);
  case OPCODE_STORE:  return get_S_imm(inst);
  case OPCODE_OP_IMM: return get_I_imm(inst);
  default: die("\nError: line ", __LINE__, ": ",
               __func__, "(", to_hex(inst), "): bad opcode");
           return -1; // unreachable!
  }
}

Instruction decode(u32 inst) {
  auto decode_OPCODE_BRANCH = [&]{
    switch (get_funct3(inst)) {
    case FUNCT3_BEQ:  return BEQ;
    case FUNCT3_BNE:  return BNE;
    case FUNCT3_BLT:  return BLT;
    case FUNCT3_BGE:  return BGE;
    case FUNCT3_BLTU: return BLTU;
    case FUNCT3_BGEU: return BGEU;
    default:          return UNDEF;
    }
  };

  auto decode_OPCODE_LOAD = [&]{
    switch (get_funct3(inst)) {
    case FUNCT3_LB:  return LB;
    case FUNCT3_LH:  return LH;
    case FUNCT3_LW:  return LW;
    case FUNCT3_LBU: return LBU;
    case FUNCT3_LHU: return LHU;
    default:         return UNDEF;
    }
  };

  auto decode_OPCODE_STORE = [&]{
    switch (get_funct3(inst)) {
    case FUNCT3_SB: return SB;
    case FUNCT3_SH: return SH;
    case FUNCT3_SW: return SW;
    default:        return UNDEF;
    }
  };

  auto decode_OPCODE_OP_IMM = [&]{
    switch (get_funct3(inst)) {
    case FUNCT3_ADDI:   return ADDI;
    case FUNCT3_SLTI:   return SLTI;
    case FUNCT3_SLTIU:  return SLTIU;
    case FUNCT3_XORI:   return XORI;
    case FUNCT3_ORI:    return ORI;
    case FUNCT3_ANDI:   return ANDI;
    case FUNCT3_SLLI:   return SLLI;
    case FUNCT3_SRAI_SRLI:
      switch (get_funct7(inst)) {
      case FUNCT7_SRAI: return SRAI;
      case FUNCT7_SRLI: return SRLI;
      default:          return UNDEF;
      }
    default:            return UNDEF;
    }
  };

  auto decode_OPCODE_OP = [&]{
    switch (get_funct3(inst)) {
    case FUNCT3_SUB_ADD:
      switch (get_funct7(inst)) {
      case FUNCT7_SUB: return SUB;
      case FUNCT7_ADD: return ADD;
      default:         return UNDEF;
      }
    case FUNCT3_SLL:   return SLL;
    case FUNCT3_SLT:   return SLT;
    case FUNCT3_SLTU:  return SLTU;
    case FUNCT3_XOR:   return XOR;
    case FUNCT3_SRA_SRL:
      switch (get_funct7(inst)) {
      case FUNCT7_SRA: return SRA;
      case FUNCT7_SRL: return SRL;
      default:         return UNDEF;
      }
    case FUNCT3_OR:    return OR;
    case FUNCT3_AND:   return AND;
    default:           return UNDEF;
    }
  };

  switch (get_opcode(inst)) {
  case OPCODE_LUI:    return LUI;
  case OPCODE_AUIPC:  return AUIPC;
  case OPCODE_JAL:    return JAL;
  case OPCODE_JALR:   return JALR;
  case OPCODE_BRANCH: return decode_OPCODE_BRANCH();
  case OPCODE_LOAD:   return decode_OPCODE_LOAD();
  case OPCODE_STORE:  return decode_OPCODE_STORE();
  case OPCODE_OP_IMM: return decode_OPCODE_OP_IMM();
  case OPCODE_OP:     return decode_OPCODE_OP();
  case OPCODE_SYSTEM: return EBREAK;
  default:            return UNDEF;
  }
}

auto disasm(auto inst) {
  static const auto regnames = std::array<std::string, 32> {
    "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9",
    "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x18", "x19",
    "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28", "x29",
    "x30", "x31"
  };

  static const auto inst_names = std::array<std::string, 39> {
    "lui", "auipc",
    "jal", "jalr", "beq", "bne", "blt", "bge", "bltu", "bgeu",
    "lb", "lh", "lw", "lbu", "lhu", "sb", "sh", "sw",
    "addi", "slti", "sltiu", "xori", "ori", "andi", "slli", "srli", "srai",
    "add", "sub", "sll", "slt", "sltu", "xor", "srl", "sra", "or", "and",
    "ebreak", "undef"
  };

  static auto imm = [&]{ return get_imm(inst); };
  static auto uimm = [&]{ return imm() >> OFFSET_U_IMM_0; };
  static auto rd  = [&]{ return str(std::setw(3), std::left, regnames[get_rd(inst)]);  };
  static auto rs1 = [&]{ return str(std::setw(3), std::left, regnames[get_rs1(inst)]); };
  static auto rs2 = [&]{ return str(std::setw(3), std::left, regnames[get_rs2(inst)]); };
  static auto addr = [&]{ return str(imm(), "(", regnames[get_rs1(inst)], ")"); };
  static auto verb = [&]{ return str(std::setw(6), std::left, inst_names[decode(inst)]); };

  switch (get_opcode(inst)) {
  case OPCODE_LUI:    return str(verb(), rd(),  " ", uimm());
  case OPCODE_AUIPC:  return str(verb(), rd(),  " ", uimm());
  case OPCODE_JAL:    return str(verb(), rd(),  " ", imm());
  case OPCODE_JALR:   return str(verb(), rd(),  " ", rs1(), " ", imm());
  case OPCODE_BRANCH: return str(verb(), rs1(), " ", rs2(), " ", imm());
  case OPCODE_LOAD:   return str(verb(), rd(),  " ", addr());
  case OPCODE_STORE:  return str(verb(), rs2(), " ", addr());
  case OPCODE_OP_IMM: return str(verb(), rd(),  " ", rs1(), " ", imm());
  case OPCODE_OP:     return str(verb(), rd(),  " ", rs1(), " ", rs2());
  case OPCODE_SYSTEM: return str(verb());
  default:            return str(verb(), to_hex(inst));
  }
}

struct Mem {
  u8* mem;
  size_t size;

  template<typename T>
  auto& operator[](auto addr) {
    if (addr + sizeof(T) - 1 >= size) {
      die("\nError: line ", __LINE__, ": ", __func__, "(", to_hex(addr), "): ",
          "invalid memory access @", to_hex(addr), '\n');
    }
    return *(T*)(mem + addr);
  }

  Mem(const char* filename) {
    auto file_size = std::filesystem::file_size(std::filesystem::path(filename));
    size = std::max(file_size, 5000000UL);
    mem = new u8[size]{0};
    auto file = std::fopen(filename, "r");
    if (std::fread(mem, sizeof(u8), file_size, file) != file_size) {
      die("fread(", filename, ") failed");
    }
    std::fclose(file);
  }

  ~Mem() {
    delete[] mem;
  }
};

struct CPU {
  std::array<u32, 33> regs = {0};

  u8* imem;
  u8* dmem;
  size_t imem_size;
  size_t dmem_size;
  u32 pc = 0;

  template<typename T>
  auto& imem_at(auto addr) {
    if (addr + sizeof(T) - 1 >= imem_size) {
      die("\nError: line ", __LINE__, ": ", __func__, "(", to_hex(addr), "): ",
          "invalid memory access @", to_hex(addr), '\n');
    }
    return *(T*)(imem + addr);
  }

  template<typename T>
  auto dmem_get(auto addr) {
    if (addr + sizeof(T) - 1 >= dmem_size) {
      die("\nError: line ", __LINE__, ": ", __func__, "(", to_hex(addr), "): ",
          "invalid memory access @", to_hex(addr), '\n');
    }
    return *(T*)(dmem + addr);
  }

  template<typename T>
  auto dmem_set(auto addr, auto x) {
    if (addr + sizeof(T) - 1 >= dmem_size) {
      die("\nError: line ", __LINE__, ": ", __func__, "(", to_hex(addr), "): ",
          "invalid memory access @", to_hex(addr), '\n');
    }
    if (addr == 0x5000) put(char(u32(x)));
    return *(T*)(dmem + addr) = x;
  }

  auto fetch() {
    auto addr = pc & 0xfffff;
    if (addr % 4) die("misaligned fetch");
    return *(u32*)(imem + addr);
  }

  auto exec(u32 inst) {
    static auto rd = [&] -> auto& { auto rd = get_rd(inst); return rd ? regs[rd] : regs[32]; };
    static auto inc_pc = [&]{ pc += 4; };
    static auto rs1 = [&]{ return regs[get_rs1(inst)]; };
    static auto rs2 = [&]{ return regs[get_rs2(inst)]; };
    static auto imm = [&]{ return get_imm(inst); };
    static auto ishamt = [&]{ return imm() & 0x1f; };
    static auto rshamt = [&]{ return rs2() & 0x1f; };
    static auto addr = [&]{ return rs1() + imm(); };
    static auto j = [&](auto target){ pc = target; };
    static auto jal = [&](auto target){ rd() = pc + 4; j(target); };
    static auto jal_target = [&]{ return pc + imm(); };
    static auto jalr_target = [&]{ return addr() & ~1; };
    static auto b_target = [&](auto cond){ return cond ? pc + imm() : pc + 4; };
    static auto branch = [&](auto cond){ return j(b_target(cond)); };
    static auto load = [&](auto x){ rd() = dmem_get<decltype(x)>(addr()); };
    static auto store = [&](auto x){ dmem_set<decltype(x)>(addr(), x); };

#define I_OP(T, OP) (((T) rs1()) OP ((T) imm()))
#define R_OP(T, OP) (((T) rs1()) OP ((T) rs2()))
#define I_SH(T, SH) (((T) rs1()) SH ((T) ishamt()))
#define R_SH(T, SH) (((T) rs1()) SH ((T) rshamt()))

    static const auto executors = std::array<std::function<void(void)>, 39> {
      /* LUI   */ [&]{ rd() = imm();         inc_pc(); },
      /* AUIPC */ [&]{ rd() = pc + imm();    inc_pc(); },
      /* JAL   */ [&]{ jal(jal_target());              },
      /* JALR  */ [&]{ jal(jalr_target());             },
      /* BEQ   */ [&]{ branch(R_OP(i32, ==));          },
      /* BNE   */ [&]{ branch(R_OP(i32, !=));          },
      /* BLT   */ [&]{ branch(R_OP(i32,  <));          },
      /* BGE   */ [&]{ branch(R_OP(i32, >=));          },
      /* BLTU  */ [&]{ branch(R_OP(u32,  <));          },
      /* BGEU  */ [&]{ branch(R_OP(u32, >=));          },
      /* LB    */ [&]{ load(i8());           inc_pc(); },
      /* LH    */ [&]{ load(i16());          inc_pc(); },
      /* LW    */ [&]{ load(u32());          inc_pc(); },
      /* LBU   */ [&]{ load(u8());           inc_pc(); },
      /* LHU   */ [&]{ load(u16());          inc_pc(); },
      /* SB    */ [&]{ store(u8(rs2()));     inc_pc(); },
      /* SH    */ [&]{ store(u16(rs2()));    inc_pc(); },
      /* SW    */ [&]{ store(u32(rs2()));    inc_pc(); },
      /* ADDI  */ [&]{ rd() = I_OP(u32,  +); inc_pc(); },
      /* SLTI  */ [&]{ rd() = I_OP(i32,  <); inc_pc(); },
      /* SLTIU */ [&]{ rd() = I_OP(u32,  <); inc_pc(); },
      /* XORI  */ [&]{ rd() = I_OP(u32,  ^); inc_pc(); },
      /* ORI   */ [&]{ rd() = I_OP(u32,  |); inc_pc(); },
      /* ANDI  */ [&]{ rd() = I_OP(u32,  &); inc_pc(); },
      /* SLLI  */ [&]{ rd() = I_SH(u32, <<); inc_pc(); },
      /* SRLI  */ [&]{ rd() = I_SH(u32, >>); inc_pc(); },
      /* SRAI  */ [&]{ rd() = I_SH(i32, >>); inc_pc(); },
      /* ADD   */ [&]{ rd() = R_OP(u32,  +); inc_pc(); },
      /* SUB   */ [&]{ rd() = R_OP(u32,  -); inc_pc(); },
      /* SLL   */ [&]{ rd() = R_SH(u32, <<); inc_pc(); },
      /* SLT   */ [&]{ rd() = R_OP(i32,  <); inc_pc(); },
      /* SLTU  */ [&]{ rd() = R_OP(u32,  <); inc_pc(); },
      /* XOR   */ [&]{ rd() = R_OP(u32,  ^); inc_pc(); },
      /* SRL   */ [&]{ rd() = R_SH(u32, >>); inc_pc(); },
      /* SRA   */ [&]{ rd() = R_SH(i32, >>); inc_pc(); },
      /* OR    */ [&]{ rd() = R_OP(u32,  |); inc_pc(); },
      /* AND   */ [&]{ rd() = R_OP(u32,  &); inc_pc(); },
      /* EBREAK*/ [&]{ log("ebreak"); exit(0); },
      /* UNDEF */ [&]{ log(disasm(inst), "  [@pc=", to_hex(pc), "]"); }
    };

    executors[decode(inst)]();
  }

  auto exec() {
    exec(fetch());
  }

  auto steps(size_t n) {
    while (n--) exec();
  }

  CPU(const auto prog_dir) {
    auto read = [&](auto& mem, auto filename, auto file_size){
      auto file = std::fopen(filename.c_str(), "r");
      if (!file) die("fopen(", filename, ") failed");
      auto read = std::fread(mem, sizeof(u8), file_size, file);
      if (read != file_size) die("fread(", filename, ") failed");
      std::fclose(file);
    };

    auto imem_filename = std::string(prog_dir) + std::string("instruction_mem.bin");
    auto dmem_filename = std::string(prog_dir) + std::string("data_mem.bin");

    try {
      auto imem_file_size = std::filesystem::file_size(std::filesystem::path(imem_filename));
      auto dmem_file_size = std::filesystem::file_size(std::filesystem::path(dmem_filename));

      imem_size = imem_file_size;
      dmem_size = std::max(dmem_file_size, 5000000UL);

      imem = new u8[imem_size]{0};
      dmem = new u8[dmem_size]{0};

      read(imem, imem_filename, imem_file_size);
      read(dmem, dmem_filename, dmem_file_size);
    } catch (std::filesystem::filesystem_error e) {
      die("could not initialize CPU memory. ", e.what());
    }
  }

  ~CPU(){
    delete[] imem;
    delete[] dmem;
  }

  auto dump_regs() {
    print();
    for (size_t i1 = 0, i2 = 16; i1 < 16; i1++, i2++) {
      print("[x", std::left, std::setw(2), i1, "] = ", to_hex(regs[i1]), "\t",
            "[x", std::left, std::setw(2), i2, "] = ", to_hex(regs[i2]));
    }
  }

  auto dump_imem(size_t n) {
    n = std::min(n, imem_size/4);
    print("\n------------------------IMEM------------------------");
    for (u32 i = 0; i < n*4; i += 4) {
      auto x = *(u32*)(imem + i);
      print("[", to_hex(i), "] = ", to_hex(x), "\t\t", disasm(x));
    }
    print("------------------------IMEM END--------------------");
  }

  auto dump_dmem(auto n) {
    n = std::min(n, dmem_size);
    print("\n------------------------DMEM------------------------");
    for (u32 i = 0; i < n; i += 4) {
      auto x = *(u32*)(dmem + i);
      print("[", to_hex(i), "] = ", to_hex(x));
    }
    print("------------------------DMEM END--------------------");
  }

};

int main(int argc, char** argv) {
  auto prog = argc >= 2 ? argv[1] : "../examples/primes/";
  auto cpu = CPU(prog);
  cpu.steps(66666666);
}
