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

template<typename T>
constexpr auto is_of(auto x) {
  return std::is_same<T, decltype(x)>::value;
}

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

using u32 = uint32_t;
using i32 = int32_t;
using u16 = uint16_t;
using i16 = int16_t;
using u8 = uint8_t;
using i8 = int8_t;

enum Instruction : size_t {
  LUI, AUIPC,
  JAL, JALR, BEQ, BNE, BLT, BGE, BLTU, BGEU,
  LB, LH, LW, LBU, LHU, SB, SH, SW,
  ADDI, SLTI, SLTIU, XORI, ORI, ANDI, SLLI, SRLI, SRAI,
  ADD, SUB, SLL, SLT, SLTU, XOR, SRL, SRA, OR, AND,
  EBREAK,
  UNDEF
};

// decoding constants
static const u32 OPCODE_LUI         = 0x37;
static const u32 OPCODE_AUIPC       = 0x17;
static const u32 OPCODE_JAL         = 0x6f;
static const u32 OPCODE_JALR        = 0x67;
static const u32 OPCODE_BRANCH      = 0x63;
static const u32 OPCODE_LOAD        = 0x03;
static const u32 OPCODE_STORE       = 0x23;
static const u32 OPCODE_OP_IMM      = 0x13;
static const u32 OPCODE_OP          = 0x33;
static const u32 OPCODE_SYSTEM      = 0x73;

#define FUNCT3_BEQ         0x0
#define FUNCT3_BNE         0x1
#define FUNCT3_BLT         0x4
#define FUNCT3_BGE         0x5
#define FUNCT3_BLTU        0x6
#define FUNCT3_BGEU        0x7
#define FUNCT3_LB          0x0
#define FUNCT3_LH          0x1
#define FUNCT3_LW          0x2
#define FUNCT3_LBU         0x4
#define FUNCT3_LHU         0x5
#define FUNCT3_SB          0x0
#define FUNCT3_SH          0x1
#define FUNCT3_SW          0x2
#define FUNCT3_ADDI        0x0
#define FUNCT3_SLTI        0x2
#define FUNCT3_SLTIU       0x3
#define FUNCT3_XORI        0x4
#define FUNCT3_ORI         0x6
#define FUNCT3_ANDI        0x7
#define FUNCT3_SLLI        0x1
#define FUNCT3_SRAI_SRLI   0x5
#define FUNCT3_SUB_ADD     0x0
#define FUNCT3_SLL         0x1
#define FUNCT3_SLT         0x2
#define FUNCT3_SLTU        0x3
#define FUNCT3_XOR         0x4
#define FUNCT3_SRA_SRL     0x5
#define FUNCT3_OR          0x6
#define FUNCT3_AND         0x7

#define FUNCT7_SRAI        0x20
#define FUNCT7_SRLI        0
#define FUNCT7_SUB         0x20
#define FUNCT7_ADD         0
#define FUNCT7_SRA         0x20
#define FUNCT7_SRL         0

// MASK_LO_HI[i] = 1 for i = LO, ..., HI else 0
#define MASK_00_06         0x0000007f
#define MASK_07_11         0x00000f80
#define MASK_15_19         0x000f8000
#define MASK_20_24         0x01f00000
#define MASK_12_14         0x00007000
#define MASK_25_31         0xfe000000
#define MASK_20_31         0xfff00000
#define MASK_07_11         0x00000f80
#define MASK_25_31         0xfe000000
#define MASK_08_11         0x00000f00
#define MASK_25_30         0x7e000000
#define MASK_07_07         0x00000080
#define MASK_31_31         0x80000000
#define MASK_12_31         0xfffff000
#define MASK_21_30         0x7fe00000
#define MASK_20_20         0x00100000
#define MASK_12_19         0x000ff000
#define MASK_31_31         0x80000000

#define MASK_OPCODE        MASK_00_06
#define MASK_RD            MASK_07_11
#define MASK_RS1           MASK_15_19
#define MASK_RS2           MASK_20_24
#define MASK_FUNCT3        MASK_12_14
#define MASK_FUNCT7        MASK_25_31
#define MASK_I_IMM_0       MASK_20_31
#define MASK_S_IMM_0       MASK_07_11
#define MASK_S_IMM_1       MASK_25_31
#define MASK_B_IMM_0       MASK_08_11
#define MASK_B_IMM_1       MASK_25_30
#define MASK_B_IMM_2       MASK_07_07
#define MASK_B_IMM_3       MASK_31_31
#define MASK_U_IMM_0       MASK_12_31
#define MASK_J_IMM_0       MASK_21_30
#define MASK_J_IMM_1       MASK_20_20
#define MASK_J_IMM_2       MASK_12_19
#define MASK_J_IMM_3       MASK_31_31

#define OFFSET_OPCODE      0
#define OFFSET_RD          7
#define OFFSET_RS1         15
#define OFFSET_RS2         20
#define OFFSET_FUNCT3      12
#define OFFSET_FUNCT7      25
#define OFFSET_I_IMM_0     20
#define OFFSET_S_IMM_0     7
#define OFFSET_S_IMM_1     25
#define OFFSET_B_IMM_0     8
#define OFFSET_B_IMM_1     25
#define OFFSET_B_IMM_2     7
#define OFFSET_B_IMM_3     31
#define OFFSET_U_IMM_0     12
#define OFFSET_J_IMM_0     21
#define OFFSET_J_IMM_1     20
#define OFFSET_J_IMM_2     12
#define OFFSET_J_IMM_3     31

// position of the immediate parts in their immediate
#define POS_I_IMM_0        0
#define POS_S_IMM_0        0
#define POS_S_IMM_1        5
#define POS_B_IMM_0        1
#define POS_B_IMM_1        5
#define POS_B_IMM_2        11
#define POS_B_IMM_3        12
#define POS_U_IMM_0        12
#define POS_J_IMM_0        1
#define POS_J_IMM_1        11
#define POS_J_IMM_2        12
#define POS_J_IMM_3        20

// sign bit position for sign extension
static const u32 SXT_BIT_I_IMM = 11;
static const u32 SXT_BIT_S_IMM = 11;
static const u32 SXT_BIT_B_IMM = 12;
static const u32 SXT_BIT_U_IMM = 31;
static const u32 SXT_BIT_J_IMM = 20;

#define SLICE(M, X)        (((X) & MASK_##M) >> OFFSET_##M)
#define GET_OPCODE(X)      SLICE(OPCODE, X)
#define GET_FUNCT3(X)      SLICE(FUNCT3, X)
#define GET_FUNCT7(X)      SLICE(FUNCT7, X)
#define GET_RD(X)          SLICE(RD, X)
#define GET_RS1(X)         SLICE(RS1, X)
#define GET_RS2(X)         SLICE(RS2, X)
#define GET_IMM(IMM, X)    SXT(SXT_BIT_##IMM##_IMM, PARTS_##IMM(X))
#define SXT(SXT_BIT, X)    ((u32)(((i32) (X) << (31 - SXT_BIT)) >> (31 - SXT_BIT)))
#define PART(IMM, PART, X) (SLICE(IMM##_IMM_##PART, X) << POS_##IMM##_IMM_##PART)
#define PARTS_I(X)         (PART(I, 0, X))
#define PARTS_S(X)         (PART(S, 0, X) | PART(S, 1, X))
#define PARTS_B(X)         (PART(B, 0, X) | PART(B, 1, X) | PART(B, 2, X) | PART(B, 3, X))
#define PARTS_U(X)         (PART(U, 0, X))
#define PARTS_J(X)         (PART(J, 0, X) | PART(J, 1, X) | PART(J, 2, X) | PART(J, 3, X))

i32 get_imm(u32 inst) {
  switch (GET_OPCODE(inst)) {
  case OPCODE_LUI:    return GET_IMM(U, inst);
  case OPCODE_AUIPC:  return GET_IMM(U, inst);
  case OPCODE_JAL:    return GET_IMM(J, inst);
  case OPCODE_JALR:   return GET_IMM(I, inst);
  case OPCODE_BRANCH: return GET_IMM(B, inst);
  case OPCODE_LOAD:   return GET_IMM(I, inst);
  case OPCODE_STORE:  return GET_IMM(S, inst);
  case OPCODE_OP_IMM: return GET_IMM(I, inst);
  default:            die("\nError: line ", __LINE__, ": ", __func__,
                          "(", to_hex(inst), "): bad opcode");
                      return -1; // unreachable!
  }
}

Instruction decode(u32 inst) {
  auto decode_OPCODE_BRANCH = [&]{
    switch (GET_FUNCT3(inst)) {
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
    switch (GET_FUNCT3(inst)) {
    case FUNCT3_LB:  return LB;
    case FUNCT3_LH:  return LH;
    case FUNCT3_LW:  return LW;
    case FUNCT3_LBU: return LBU;
    case FUNCT3_LHU: return LHU;
    default:         return UNDEF;
    }
  };

  auto decode_OPCODE_STORE = [&]{
    switch (GET_FUNCT3(inst)) {
    case FUNCT3_SB: return SB;
    case FUNCT3_SH: return SH;
    case FUNCT3_SW: return SW;
    default:        return UNDEF;
    }
  };

  auto decode_OPCODE_OP_IMM = [&]{
    switch (GET_FUNCT3(inst)) {
    case FUNCT3_ADDI:   return ADDI;
    case FUNCT3_SLTI:   return SLTI;
    case FUNCT3_SLTIU:  return SLTIU;
    case FUNCT3_XORI:   return XORI;
    case FUNCT3_ORI:    return ORI;
    case FUNCT3_ANDI:   return ANDI;
    case FUNCT3_SLLI:   return SLLI;
    case FUNCT3_SRAI_SRLI:
      switch (GET_FUNCT7(inst)) {
      case FUNCT7_SRAI: return SRAI;
      case FUNCT7_SRLI: return SRLI;
      default:          return UNDEF;
      }
    default:            return UNDEF;
    }
  };

  auto decode_OPCODE_OP = [&]{
    switch (GET_FUNCT3(inst)) {
    case FUNCT3_SUB_ADD:
      switch (GET_FUNCT7(inst)) {
      case FUNCT7_SUB: return SUB;
      case FUNCT7_ADD: return ADD;
      default:         return UNDEF;
      }
    case FUNCT3_SLL:   return SLL;
    case FUNCT3_SLT:   return SLT;
    case FUNCT3_SLTU:  return SLTU;
    case FUNCT3_XOR:   return XOR;
    case FUNCT3_SRA_SRL:
      switch (GET_FUNCT7(inst)) {
      case FUNCT7_SRA: return SRA;
      case FUNCT7_SRL: return SRL;
      default:         return UNDEF;
      }
    case FUNCT3_OR:    return OR;
    case FUNCT3_AND:   return AND;
    default:           return UNDEF;
    }
  };

  switch (GET_OPCODE(inst)) {
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

auto is_in(auto x, auto... args) {
  return ((x == args) || ... );
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
  const auto opcode = GET_OPCODE(inst);
  static auto offset = [&]{ return is_in(opcode, OPCODE_LUI, OPCODE_AUIPC) ? OFFSET_U_IMM_0 : 0; };
  static auto imm = [&]{ return get_imm(inst) >> offset(); };
  static auto rd  = [&]{ return str(std::setw(3), std::left, regnames[GET_RD(inst)]);  };
  static auto rs1 = [&]{ return str(std::setw(3), std::left, regnames[GET_RS1(inst)]); };
  static auto rs2 = [&]{ return str(std::setw(3), std::left, regnames[GET_RS2(inst)]); };
  static auto addr = [&]{ return str(imm(), "(", regnames[GET_RS1(inst)], ")"); };
  static auto verb = [&]{ return str(std::setw(6), std::left, inst_names[decode(inst)]); };
  static auto dis = [&](auto... args) { return str(verb(), args...); };

  switch (opcode) {
  case OPCODE_LUI:    return dis(rd(),  " ", imm());
  case OPCODE_AUIPC:  return dis(rd(),  " ", imm());
  case OPCODE_JAL:    return dis(rd(),  " ", imm());
  case OPCODE_JALR:   return dis(rd(),  " ", rs1(), " ", imm());
  case OPCODE_BRANCH: return dis(rs1(), " ", rs2(), " ", imm());
  case OPCODE_LOAD:   return dis(rd(),  " ", addr());
  case OPCODE_STORE:  return dis(rs2(), " ", addr());
  case OPCODE_OP_IMM: return dis(rd(),  " ", rs1(), " ", imm());
  case OPCODE_OP:     return dis(rd(),  " ", rs1(), " ", rs2());
  case OPCODE_SYSTEM: return dis();
  default:            return dis(to_hex(inst));
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
    static auto rd = [&] -> auto& { auto rd = GET_RD(inst); return rd ? regs[rd] : regs[32]; };
    static auto inc_pc = [&]{ pc += 4; };
    static auto rs1 = [&]{ return regs[GET_RS1(inst)]; };
    static auto rs2 = [&]{ return regs[GET_RS2(inst)]; };
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
