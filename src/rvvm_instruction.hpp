enum class Opcode {
  LUI      = 0b011'0111,  // type U
  AUIPC    = 0b001'0111,  // type U
  JAL      = 0b110'1111,  // type J
  JALR     = 0b110'0111,  // type I
  BRANCH   = 0b110'0011,  // type B : BEQ, BNE, BLT, BGE, BLTU, BGEU
  LOAD     = 0b000'0011,  // type I : LW,...
  STORE    = 0b010'0011,  // type S
  IMM      = 0b001'0011,  // type I : ADDI, SLTI,...,SLLI,...,SRAI
  OP       = 0b011'0011,  // type R : ADD,..., AND
  MISC_MEM = 0b000'1111,  // FENCE, FENCE_TSO, PAUSE
  SYSTEM   = 0b111'0011,  // type I : ECALL, EBREAK ?

  NOIMPL,
};

enum class Operation {
  LUI,
  AUIPC,
  JAL,
  JALR,
  BEQ,
  BNE,
  BLT,
  BGE,
  BLTU,
  BGEU,
  LB,
  LH,
  LW,
  LBU,
  LHU,
  SB,
  SH,
  SW,
  ADDI,
  SLTI,
  SLTIU,
  XORI,
  ORI,
  ANDI,
  SLLI,
  SRLI,
  SRAI,
  ADD,
  SUB,
  SLL,
  SLT,
  SLTU,
  XOR,
  SRL,
  SRA,
  OR,
  AND,
  FENCE,
  FENCE_TSO,
  PAUSE,
  ECALL,
  EBREAK,

  DECODE_ERR,
  NOIMPL,
};


enum class InstructionType {
  R,
  I,
  S,
  B,
  U,
  J,
};

struct Instruction : Word {
  word            w;
  Opcode          opcode;
  Operation       operation;
  word            rd;
  word            rs1;
  word            rs2;
  word            funct3;
  word            funct7;
  InstructionType type;
  word            imm;

  inline auto get_imm() {
    switch(type) {
      case InstructionType::I: return get_slice(w, 20, 31);
      case InstructionType::S: return (get_slice(w, 25, 31) << 5) | get_slice(w, 7, 11);
      case InstructionType::B: return word(0);
      case InstructionType::U: return get_slice(w, 12, 31) << 12;
      case InstructionType::J: return word(0);
      default:                 return word(0);
    }
  }

  inline auto get_opcode    () { return Opcode(get_slice(w,  0,  6)); }
  inline auto get_rd        () { return get_slice(w,  7, 11); }
  inline auto get_rs1       () { return get_slice(w, 15, 19); }
  inline auto get_rs2       () { return get_slice(w, 20, 24); }
  inline auto get_funct3    () { return get_slice(w, 12, 14); }
  inline auto get_funct7    () { return get_slice(w, 25, 31); }

  inline auto get_type      () {
    switch (get_opcode()) {
      case Opcode::LUI      : return InstructionType::U;
      case Opcode::AUIPC    : return InstructionType::U;
      case Opcode::JAL      : return InstructionType::J;
      case Opcode::JALR     : return InstructionType::I;
      case Opcode::BRANCH   : return InstructionType::B;
      case Opcode::LOAD     : return InstructionType::I;
      case Opcode::STORE    : return InstructionType::S;
      case Opcode::IMM      : return InstructionType::I;
      case Opcode::OP       : return InstructionType::R;
      case Opcode::MISC_MEM : return InstructionType::I;
      case Opcode::SYSTEM   : return InstructionType::I;

      case Opcode::NOIMPL   : // fall through
      default:                return InstructionType::I;
    }
  }

  auto set_imm(word v) {
    switch(type) {
      case InstructionType::R: return word(0);
      case InstructionType::I: return set_slice(w, v, 20, 31);;
      case InstructionType::S: return (get_slice(w, 25, 31) << 5) | get_slice(w, 7, 11);
      case InstructionType::B: return word(0);
      case InstructionType::U: return get_slice(w, 12, 31) << 12;
      case InstructionType::J: return word(0);
      default                : return word(0);
    }
  }

  inline auto set_opcode    (word v) { return set_slice(w, v,  0,  6); }
  inline auto set_rd        (word v) { return set_slice(w, v,  7, 11); }
  inline auto set_rs1       (word v) { return set_slice(w, v, 15, 19); }
  inline auto set_rs2       (word v) { return set_slice(w, v, 20, 24); }
  inline auto set_funct3    (word v) { return set_slice(w, v, 12, 14); }
  inline auto set_funct7    (word v) { return set_slice(w, v, 25, 31); }
  inline auto set_type      (word v) { return set_slice(w, v, 25, 31); }
  inline auto set_operation (word v) {}

  auto get_operation() {
    switch (get_opcode()) {

      case Opcode::LUI:   return Operation::LUI;
      case Opcode::AUIPC: return Operation::AUIPC;
      case Opcode::JAL:   return Operation::JAL;
      case Opcode::JALR:  return Operation::JALR;

      case Opcode::BRANCH: 
        switch (get_funct3()) {
          case 0b000: return Operation::BEQ;
          case 0b001: return Operation::BNE;
          case 0b100: return Operation::BLT;
          case 0b101: return Operation::BGE;
          case 0b110: return Operation::BLTU;
          case 0b111: return Operation::BGEU;
          default:    return Operation::DECODE_ERR;
        }

      case Opcode::LOAD:
        switch (get_funct3()) {
          case 0b000: return Operation::LB;
          case 0b001: return Operation::LH;
          case 0b010: return Operation::LW;
          case 0b100: return Operation::LBU;
          case 0b101: return Operation::LHU;
          default:    return Operation::DECODE_ERR;
        }

      case Opcode::STORE:
        switch (get_funct3()) {
          case 0b000: return Operation::SB;
          case 0b001: return Operation::SH;
          case 0b010: return Operation::SW;
          default:    return Operation::DECODE_ERR;
        }

      case Opcode::IMM:
        switch (get_funct3()) {
          case 0b000: return Operation::ADDI;
          case 0b010: return Operation::SLTI;
          case 0b011: return Operation::SLTIU;
          case 0b100: return Operation::XORI;
          case 0b110: return Operation::ORI;
          case 0b111: return Operation::ANDI;
          case 0b001: return Operation::SLLI;
          case 0b101: return get_funct7() ? Operation::SRAI : Operation::SRLI;
          default:    return Operation::DECODE_ERR;
        }

      case Opcode::OP:
        switch (get_funct3()) {
          case 0b000: return get_funct7() ? Operation::SUB : Operation::ADD;
          case 0b001: return Operation::SLL;
          case 0b010: return Operation::SLT;
          case 0b011: return Operation::SLTU;
          case 0b100: return Operation::XOR;
          case 0b101: return get_funct7() ? Operation::SRA : Operation::SRL;
          case 0b110: return Operation::OR;
          case 0b111: return Operation::AND;
          default:    return Operation::DECODE_ERR;
        }

      case Opcode::MISC_MEM: return Operation::NOIMPL;
      case Opcode::SYSTEM:   return get_imm() ? Operation::EBREAK : Operation::ECALL;
      default:               return Operation::DECODE_ERR;
    }
  }

  Instruction(Word _w) {
    w         = _w.w;
    opcode    = get_opcode();
    operation = get_operation();
    rd        = get_rd();
    rs1       = get_rs1();
    rs2       = get_rs2();
    funct3    = get_funct3();
    funct7    = get_funct7();
    type      = get_type();
    imm       = get_imm();
  }

  Instruction(String line) {
    w         = word(std::stoi(line));
    opcode    = get_opcode();
    operation = get_operation();
    rd        = get_rd();
    rs1       = get_rs1();
    rs2       = get_rs2();
    funct3    = get_funct3();
    funct7    = get_funct7();
    type      = get_type();
    imm       = get_imm();
  }
};
