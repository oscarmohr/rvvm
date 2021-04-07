struct CPU {
  word      pc;
  Registers regs;
  Memory    mem;

  auto get_reg(word reg) {
    if (reg >= 32) die("Invalid register index: ", reg);
    return regs[reg];
  }

  auto set_reg(word reg, word value) {
    if (reg >= 32) die("Invalid register index: ", reg);
    regs[reg] = (reg ? value : 0);
  }

  auto get_mem(word m) { return mem[m]; }

  auto get_mem_word(word m) {
    // little endian
    word byte0 = mem[m + 0] << 24;
    word byte1 = mem[m + 1] << 16;
    word byte2 = mem[m + 2] <<  8;
    word byte3 = mem[m + 3] <<  0;
    return byte0 & byte1 & byte2 & byte3;
  }

  auto set_mem(word m, word value) { mem[m] = value; }

  // execution of decoded instructions
  auto exec_LUI       (word w) { }
  auto exec_AUIPC     (word w) { }
  auto exec_JAL       (word w) { }
  auto exec_JALR      (word w) { }
  auto exec_BEQ       (word w) { }
  auto exec_BNE       (word w) { }
  auto exec_BLT       (word w) { }
  auto exec_BGE       (word w) { }
  auto exec_BLTU      (word w) { }
  auto exec_BGEU      (word w) { }
  auto exec_LB        (word w) {
    word base   = get_reg(get_rs1(w));
    word offset = get_typeI_imm(w);
    word addr   = base + offset;
    word dest   = get_rd(w);
    word src    = get_mem(addr);
    // sign extend src
    /* src & (1 << 31) ? 1:0; */
    set_reg(dest, src);
  }
  auto exec_LH        (word w) {
    word imm = get_typeI_imm(w);
  }
  auto exec_LW        (word w) {
    word imm = get_typeI_imm(w);
  }
  auto exec_LBU       (word w) {
    word imm = get_typeI_imm(w);
  }
  auto exec_LHU       (word w) {
    word imm = get_typeI_imm(w);
  }
  // sign extension ???
  auto exec_SB        (word w) {
    word base   = get_reg(get_rs1(w));
    word offset = get_typeS_imm(w);
    word dest   = base + offset;
    word value  = get_slice(get_reg(get_rs2(w)), 0, 7); // get byte
    set_mem(dest, value);
  }
  auto exec_SH        (word w) {
    word base   = get_reg(get_rs1(w));
    word offset = get_typeS_imm(w);
    word dest   = base + offset;
    word value  = get_slice(get_reg(get_rs2(w)), 0, 15); // get hword
    set_mem(dest, value);
  }
  auto exec_SW        (word w) {
    word base = get_reg(get_rs1(w));
    word offset = get_typeS_imm(w);
    word dest = base + offset;
    word value = get_reg(get_rs2(w)); // get word
    set_mem(dest, value);
  }//??sign extension
  auto exec_ADDI      (word w) { }
  auto exec_SLTI      (word w) { }
  auto exec_SLTIU     (word w) { }
  auto exec_XORI      (word w) { }
  auto exec_ORI       (word w) { }
  auto exec_ANDI      (word w) { }
  auto exec_SLLI      (word w) { }
  auto exec_SRLI      (word w) { }
  auto exec_SRAI      (word w) { }
  auto exec_ADD       (word w) {
    word dest = get_rd(w);
    word op1  = get_reg(get_rs1(w));
    word op2  = get_reg(get_rs2(w));
    word res  = op1 + op2;
    set_reg(dest, res);
  }
  auto exec_SUB       (word w) {
    word dest = get_rd(w);
    word op1  = get_reg(get_rs1(w));
    word op2  = get_reg(get_rs2(w));
    word res  = op1 - op2;
    set_reg(dest, res);
  }
  auto exec_SLL       (word w) {
    word dest = get_rd(w);
    word op1  = get_reg(get_rs1(w));
    word op2  = get_slice(get_reg(get_rs2(w)), 0, 4);
    word res  = op1 << op2;
    set_reg(dest, res);
  }
  auto exec_SLT       (word w) {
    word dest = get_rd(w);
    word op1  = get_reg(get_rs1(w));
    word op2  = get_reg(get_rs2(w));
    word res  = (signed) op1 < (signed) op2 ? 1 : 0;
    set_reg(dest, res);
  }
  auto exec_SLTU      (word w) {
    word dest = get_rd(w);
    word op1  = get_reg(get_rs1(w));
    word op2  = get_reg(get_rs2(w));
    word res  = op1 < op2 ? 1 : 0;
    set_reg(dest, res);
  }
  auto exec_XOR       (word w) {
    word dest = get_rd(w);
    word op1  = get_reg(get_rs1(w));
    word op2  = get_reg(get_rs2(w));
    word res  = op1 ^ op2;
    set_reg(dest, res);
  }
  auto exec_SRL       (word w) {
    word dest = get_rd(w);
    word op1  = get_reg(get_rs1(w));
    word op2  = get_slice(get_reg(get_rs2(w)), 0, 4);
    word res  = op1 >> op2;
    set_reg(dest, res);
  }
  auto exec_SRA       (word w) {
    word dest = get_rd(w);
    word op1  = get_reg(get_rs1(w));
    word op2  = get_slice(get_reg(get_rs2(w)), 0, 4);
    word res  = op1 >> op2;
    if (op1 & (1 << 31))
      res |= (1 << 31);
    set_reg(dest, res);
  }
  auto exec_OR        (word w) { 
    word dest = get_rd(w);
    word op1  = get_reg(get_rs1(w));
    word op2  = get_reg(get_rs2(w));
    word res  = op1 | op2;
    set_reg(dest, res);
  }
  auto exec_AND       (word w) {
    word dest = get_rd(w);
    word op1  = get_reg(get_rs1(w));
    word op2  = get_reg(get_rs2(w));
    word res  = op1 & op2;
    set_reg(dest, res);
  }
  auto exec_FENCE     (word w) { }
  auto exec_FENCE_TSO (word w) { }
  auto exec_PAUSE     (word w) { }
  auto exec_ECALL     (word w) { }
  auto exec_EBREAK    (word w) { }

  auto exec(word w) {
    switch (get_operation(w)) {
      case  LUI       : return exec_LUI       (w);
      case  AUIPC     : return exec_AUIPC     (w);
      case  JAL       : return exec_JAL       (w);
      case  JALR      : return exec_JALR      (w);
      case  BEQ       : return exec_BEQ       (w);
      case  BNE       : return exec_BNE       (w);
      case  BLT       : return exec_BLT       (w);
      case  BGE       : return exec_BGE       (w);
      case  BLTU      : return exec_BLTU      (w);
      case  BGEU      : return exec_BGEU      (w);
      case  LB        : return exec_LB        (w);
      case  LH        : return exec_LH        (w);
      case  LW        : return exec_LW        (w);
      case  LBU       : return exec_LBU       (w);
      case  LHU       : return exec_LHU       (w);
      case  SB        : return exec_SB        (w);
      case  SH        : return exec_SH        (w);
      case  SW        : return exec_SW        (w);
      case  ADDI      : return exec_ADDI      (w);
      case  SLTI      : return exec_SLTI      (w);
      case  SLTIU     : return exec_SLTIU     (w);
      case  XORI      : return exec_XORI      (w);
      case  ORI       : return exec_ORI       (w);
      case  ANDI      : return exec_ANDI      (w);
      case  SLLI      : return exec_SLLI      (w);
      case  SRLI      : return exec_SRLI      (w);
      case  SRAI      : return exec_SRAI      (w);
      case  ADD       : return exec_ADD       (w);
      case  SUB       : return exec_SUB       (w);
      case  SLL       : return exec_SLL       (w);
      case  SLT       : return exec_SLT       (w);
      case  SLTU      : return exec_SLTU      (w);
      case  XOR       : return exec_XOR       (w);
      case  SRL       : return exec_SRL       (w);
      case  SRA       : return exec_SRA       (w);
      case  OR        : return exec_OR        (w);
      case  AND       : return exec_AND       (w);
      case  FENCE     : return exec_FENCE     (w);
      case  FENCE_TSO : return exec_FENCE_TSO (w);
      case  PAUSE     : return exec_PAUSE     (w);
      case  ECALL     : return exec_ECALL     (w);
      case  EBREAK    : return exec_EBREAK    (w);
      default         : die("Invalid Operation.");
    }
  }

  auto print() {
    std::cout << "==================== pc>        ====================" << '\n';
    std::cout << "pc \t= ";
    print_bits(pc);
    std::cout << "==================== <pc        ====================" << '\n';

    regs.print(4);
    mem.print();
  }

  auto run() {
    while (true) exec(mem[pc]);
  }
};
