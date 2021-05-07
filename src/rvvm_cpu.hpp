using namespace olib;

struct Cpu {
  word      pc;
  Registers regs;
  Memory    mem;

  inline auto next_instruction() { return Instruction(mem[pc]); }

  inline auto exec_LUI        (Instruction w) {
    regs[w.rd] = w.imm;
  }

  inline auto exec_AUIPC      (Instruction w) { }

  inline auto exec_JAL        (Instruction w) { }

  inline auto exec_JALR       (Instruction w) { }

  inline auto exec_BEQ        (Instruction w) { }

  inline auto exec_BNE        (Instruction w) { }

  inline auto exec_BLT        (Instruction w) { }

  inline auto exec_BGE        (Instruction w) { }

  inline auto exec_BLTU       (Instruction w) { }

  inline auto exec_BGEU       (Instruction w) { }

  inline auto exec_LB         (Instruction w) {
    auto& rd = regs[w.rd];
    auto rs1 = regs[w.rs1];
    auto offset = w.imm;
    // sxt
    rd = mem[rs1] + offset;
    regs[w.rd] = mem[regs[w.rs1] + w.imm];
    /* word base   = get_reg(get_rs1(w)); */
    /* word offset = get_typeI_imm(w); */
    /* word addr   = base + offset; */
    /* word dest   = get_rd(w); */
    /* word src    = get_mem(addr); */
    /* // sign extend src */
    /* /1* src & (1 << 31) ? 1:0; *1/ */
    /* set_reg(dest, src); */
  }

  inline auto exec_LH         (Instruction w) { }

  inline auto exec_LW         (Instruction w) { }

  inline auto exec_LBU        (Instruction w) { }

  inline auto exec_LHU        (Instruction w) { }

  inline auto exec_SB         (Instruction w) {
    /* word base   = get_reg(get_rs1(w)); */
    /* word offset = get_typeS_imm(w); */
    /* word dest   = base + offset; */
    /* word value  = get_slice(get_reg(get_rs2(w)), 0, 7); // get byte */
    /* set_mem(dest, value); */
  }

  inline auto exec_SH         (Instruction w) {
    /* word base   = get_reg(get_rs1(w)); */
    /* word offset = get_typeS_imm(w); */
    /* word dest   = base + offset; */
    /* word value  = get_slice(get_reg(get_rs2(w)), 0, 15); // get hword */
    /* set_mem(dest, value); */
  }

  inline auto exec_SW         (Instruction w) {
    /* word base = get_reg(get_rs1(w)); */
    /* word offset = get_typeS_imm(w); */
    /* word dest = base + offset; */
    /* word value = get_reg(get_rs2(w)); // get word */
    /* set_mem(dest, value); */
  }

  inline auto exec_ADDI       (Instruction w) { }

  inline auto exec_SLTI       (Instruction w) { }

  inline auto exec_SLTIU      (Instruction w) { }

  inline auto exec_XORI       (Instruction w) { }

  inline auto exec_ORI        (Instruction w) { }

  inline auto exec_ANDI       (Instruction w) { }

  inline auto exec_SLLI       (Instruction w) { }

  inline auto exec_SRLI       (Instruction w) { }

  inline auto exec_SRAI       (Instruction w) { }

  inline auto exec_ADD        (Instruction w) {
    // x[w.rd] = x[w.rs1] + x[w.rs2];
  }

  inline auto exec_SUB        (Instruction w) {
    // x[w.rd] = x[w.rs1] - x[w.rs2];
  }

  inline auto exec_SLL        (Instruction w) {
    // x[w.rd] = x[w.rs1] << x[w.rs2](0, 4);
  }

  inline auto exec_SLT        (Instruction w) {
    // x[w.rd] = (signed) x[w.rs1] < (signed) x[w.rs2] ? 1 : 0;
  }

  inline auto exec_SLTU       (Instruction w) {
    auto rd  = regs[w.rd];
    auto rs1 = regs[w.rs1];
    auto rs2 = regs[w.rs2];
    /* word dest = get_rd(w); */
    /* word op1  = get_reg(get_rs1(w)); */
    /* word op2  = get_reg(get_rs2(w)); */
    /* word res  = op1 < op2 ? 1 : 0; */
    /* set_reg(dest, res); */
  }

  inline auto exec_XOR        (Instruction w) {
    auto rd  = regs[w.rd];
    auto rs1 = regs[w.rs1];
    auto rs2 = regs[w.rs2];
    /* word dest = get_rd(w); */
    /* word op1  = get_reg(get_rs1(w)); */
    /* word op2  = get_reg(get_rs2(w)); */
    /* word res  = op1 ^ op2; */
    /* set_reg(dest, res); */
  }

  inline auto exec_SRL        (Instruction w) {
    auto rd    = regs[w.rd];
    auto rs1   = regs[w.rs1];
    auto rs2   = regs[w.rs2];
    auto shamt = rs2.at(0, 4);

    rd = rs1 >> shamt;
  }

  inline auto exec_SRA        (Instruction w) {
    auto rd    = regs[w.rd];
    auto rs1   = regs[w.rs1];
    auto rs2   = regs[w.rs2];
    auto shamt = rs2.at(0, 4);

    rd = rs1 >> shamt;

    /* word dest = get_rd(w); */
    /* word op1  = get_reg(get_rs1(w)); */
    /* word op2  = get_slice(get_reg(get_rs2(w)), 0, 4); */
    /* word res  = op1 >> op2; */
    /* if (op1 & (1 << 31)) */
    /* res |= (1 << 31); */
    /* set_reg(dest, res); */
  }

  inline auto exec_OR         (Instruction w) {
    instruction = w;
    rd = rs1 | rs2;
  }

  inline auto exec_AND        (Instruction w) {
    auto& rd  = regs[w.rd];
    auto  rs1 = regs[w.rs1];
    auto  rs2 = regs[w.rs2];

    rd = rs1 & rs2;
  }

  inline auto exec_FENCE      (Instruction w) { }

  inline auto exec_FENCE_TSO  (Instruction w) { }

  inline auto exec_PAUSE      (Instruction w) { }

  inline auto exec_ECALL      (Instruction w) { }

  inline auto exec_EBREAK     (Instruction w) { }

  inline auto exec_DECODE_ERR (Instruction w) { }

  inline auto exec_NOIMPL     (Instruction w) { }

  auto exec(Instruction w) {
    switch (w.operation) {
      case Operation::LUI        : return exec_LUI        (w);
      case Operation::AUIPC      : return exec_AUIPC      (w);
      case Operation::JAL        : return exec_JAL        (w);
      case Operation::JALR       : return exec_JALR       (w);
      case Operation::BEQ        : return exec_BEQ        (w);
      case Operation::BNE        : return exec_BNE        (w);
      case Operation::BLT        : return exec_BLT        (w);
      case Operation::BGE        : return exec_BGE        (w);
      case Operation::BLTU       : return exec_BLTU       (w);
      case Operation::BGEU       : return exec_BGEU       (w);
      case Operation::LB         : return exec_LB         (w);
      case Operation::LH         : return exec_LH         (w);
      case Operation::LW         : return exec_LW         (w);
      case Operation::LBU        : return exec_LBU        (w);
      case Operation::LHU        : return exec_LHU        (w);
      case Operation::SB         : return exec_SB         (w);
      case Operation::SH         : return exec_SH         (w);
      case Operation::SW         : return exec_SW         (w);
      case Operation::ADDI       : return exec_ADDI       (w);
      case Operation::SLTI       : return exec_SLTI       (w);
      case Operation::SLTIU      : return exec_SLTIU      (w);
      case Operation::XORI       : return exec_XORI       (w);
      case Operation::ORI        : return exec_ORI        (w);
      case Operation::ANDI       : return exec_ANDI       (w);
      case Operation::SLLI       : return exec_SLLI       (w);
      case Operation::SRLI       : return exec_SRLI       (w);
      case Operation::SRAI       : return exec_SRAI       (w);
      case Operation::ADD        : return exec_ADD        (w);
      case Operation::SUB        : return exec_SUB        (w);
      case Operation::SLL        : return exec_SLL        (w);
      case Operation::SLT        : return exec_SLT        (w);
      case Operation::SLTU       : return exec_SLTU       (w);
      case Operation::XOR        : return exec_XOR        (w);
      case Operation::SRL        : return exec_SRL        (w);
      case Operation::SRA        : return exec_SRA        (w);
      case Operation::OR         : return exec_OR         (w);
      case Operation::AND        : return exec_AND        (w);
      case Operation::FENCE      : return exec_FENCE      (w);
      case Operation::FENCE_TSO  : return exec_FENCE_TSO  (w);
      case Operation::PAUSE      : return exec_PAUSE      (w);
      case Operation::ECALL      : return exec_ECALL      (w);
      case Operation::EBREAK     : return exec_EBREAK     (w);

      case Operation::DECODE_ERR : return exec_DECODE_ERR (w);
      case Operation::NOIMPL     : return exec_NOIMPL     (w);
    }
  }

  auto exec() {
    exec(next_instruction());
  }

  auto exec_steps(u32 steps) {
    for (auto i : range(steps))
      exec();
  }

  auto print() {
    olib::print("====================================================");
    olib::print("==================== cpu>       ====================");

    put("pc      = ");
    print_bits(pc);

    put("mem[pc] = ");
    print_bits((word) mem[pc]);

    regs.print(4);
    mem.print();

    olib::print("==================== <cpu       ====================");
    olib::print("====================================================");
    put("\n");
  }
};
