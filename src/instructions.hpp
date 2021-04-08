enum Opcode {
  OP_LUI      = 0b011'0111,  // type U
  OP_AUIPC    = 0b001'0111,  // type U
  OP_JAL      = 0b110'1111,  // type J
  OP_JALR     = 0b110'0111,  // type I
  OP_BRANCH   = 0b110'0011,  // type B : BEQ, BNE, BLT, BGE, BLTU, BGEU
  OP_LOAD     = 0b000'0011,  // type I : LW,...
  OP_STORE    = 0b010'0011,  // type S
  OP_IMM      = 0b001'0011,  // type I : ADDI, SLTI,...,SLLI,...,SRAI
  OP_OP       = 0b011'0011,  // type R : ADD,..., AND
  OP_MISC_MEM = 0b000'1111,  // FENCE, FENCE_TSO, PAUSE
  OP_SYSTEM   = 0b111'0011,  // ECALL, EBREAK
  OP_NONE
};

enum Operation {
  LUI       ,
  AUIPC     ,
  JAL       ,
  JALR      ,
  BEQ       ,
  BNE       ,
  BLT       ,
  BGE       ,
  BLTU      ,
  BGEU      ,
  LB        ,
  LH        ,
  LW        ,
  LBU       ,
  LHU       ,
  SB        ,
  SH        ,
  SW        ,
  ADDI      ,
  SLTI      ,
  SLTIU     ,
  XORI      ,
  ORI       ,
  ANDI      ,
  SLLI      ,
  SRLI      ,
  SRAI      ,
  ADD       ,
  SUB       ,
  SLL       ,
  SLT       ,
  SLTU      ,
  XOR       ,
  SRL       ,
  SRA       ,
  OR        ,
  AND       ,
  FENCE     ,
  FENCE_TSO ,
  PAUSE     ,
  ECALL     ,
  EBREAK    ,

  DECODE_ERR,
  NOIMPL,
};

struct Instruction {
  word rd;
  word rs1;
  word rs2;
  word opcode;
  word funct3;
  word funct7;
  word imm_typeI;
  word imm_typeS;

  word type;

  Instruction(word w) {

  }
};

// get and set specific parts of instructions
inline auto get_opcode (word  w)         { return get_slice(w,     0,  6); }
inline auto get_rd     (word  w)         { return get_slice(w,     7, 11); }
inline auto get_funct3 (word  w)         { return get_slice(w,    12, 14); }
inline auto get_rs1    (word  w)         { return get_slice(w,    15, 19); }
inline auto get_rs2    (word  w)         { return get_slice(w,    20, 24); }
inline auto get_funct7 (word  w)         { return get_slice(w,    25, 31); }

inline auto set_opcode (word& w, word v) { return set_slice(w, v,  0,  6); }
inline auto set_rd     (word& w, word v) { return set_slice(w, v,  7, 11); }
inline auto set_funct3 (word& w, word v) { return set_slice(w, v, 12, 14); }
inline auto set_rs1    (word& w, word v) { return set_slice(w, v, 15, 19); }
inline auto set_rs2    (word& w, word v) { return set_slice(w, v, 20, 24); }
inline auto set_funct7 (word& w, word v) { return set_slice(w, v, 25, 31); }

inline auto get_typeI_imm(word w) {
  word imm = get_slice(w, 20, 31);
  return imm;
}

inline auto get_typeS_imm(word w) {
  word imm_u = get_slice(w, 25, 31) << 5;
  word imm_l = get_slice(w, 7, 11);
  return imm_u | imm_l;
}

inline auto get_typeB_imm(word w);

inline auto get_typeU_imm(word w) { return get_slice(w, 12, 31) << 12; }

inline auto get_typeJ_imm(word w);

auto get_operation(word w) {
  switch (get_opcode(w)) {
    case OP_LUI:     return LUI;
    case OP_AUIPC:   return AUIPC;
    case OP_JAL:     return JAL;
    case OP_JALR:    return JALR;

    case OP_BRANCH: 
      switch (get_funct3(w)) {
        case 0b000 : return BEQ;
        case 0b001 : return BNE;
        case 0b100 : return BLT;
        case 0b101 : return BGE;
        case 0b110 : return BLTU;
        case 0b111 : return BGEU;
        default    : return DECODE_ERR;
      }

    case OP_LOAD:
      switch (get_funct3(w)) {
        case 0b000 : return LB;
        case 0b001 : return LH;
        case 0b010 : return LW;
        case 0b100 : return LBU;
        case 0b101 : return LHU;
        default    : return DECODE_ERR;
      }

    case OP_STORE:
      switch (get_funct3(w)) {
        case 0b000 : return SB;
        case 0b001 : return SH;
        case 0b010 : return SW;
        default    : return DECODE_ERR;
      }

    case OP_IMM:
      switch (get_funct3(w)) {
        case 0b000 : return ADDI;
        case 0b010 : return SLTI;
        case 0b011 : return SLTIU;
        case 0b100 : return XORI;
        case 0b110 : return ORI;
        case 0b111 : return ANDI;
        case 0b001 : return SLLI;
        case 0b101 : return get_funct7(w) ? SRAI : SRLI;
        default    : return DECODE_ERR;
      }

    case OP_OP:
      switch (get_funct3(w)) {
        case 0b000 : return get_funct7(w) ? SUB : ADD;
        case 0b001 : return SLL;
        case 0b010 : return SLT;
        case 0b011 : return SLTU;
        case 0b100 : return XOR;
        case 0b101 : return get_funct7(w) ? SRA : SRL;
        case 0b110 : return OR;
        case 0b111 : return AND;
        default    : return DECODE_ERR;
      }

    case OP_MISC_MEM : return NOIMPL;
    case OP_SYSTEM   : return get_typeI_imm(w) ? EBREAK : ECALL;
    default          : return DECODE_ERR;
  }
}
