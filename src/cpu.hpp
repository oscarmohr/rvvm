struct CPU {
  word      pc;
  Registers regs;
  Memory    mem;

  auto exec(word w) {
    switch (get_operation(w)) {
      case  LUI:
        break;

      case  AUIPC:
        break;

      case  JAL:
        break;

      case  JALR:
        break;

      case  BEQ:
        break;

      case  BNE:
        break;

      case  BLT:
        break;

      case  BGE:
        break;

      case  BLTU:
        break;

      case  BGEU:
        break;

      case  LB:
        word base   = get_reg(get_rs1(w));
        word offset = get_typeI_imm(w);
        word addr   = base + offset;
        word dest   = get_rd(w);
        word src    = get_mem(addr);
        // sign extend src
        /* src & (1 << 31) ? 1:0; */
        set_reg(dest, src);
        break;

      case  LH:
        break;

      case  LW:
        break;

      case  LBU:
        break;

      case  LHU:
        break;

      case  SB:
        word base   = get_reg(get_rs1(w));
        word offset = get_typeS_imm(w);
        word dest   = base + offset;
        word value  = get_slice(get_reg(get_rs2(w)), 0, 7); // get byte
        set_mem(dest, value);
        break;

      case  SH:
        word base   = get_reg(get_rs1(w));
        word offset = get_typeS_imm(w);
        word dest   = base + offset;
        word value  = get_slice(get_reg(get_rs2(w)), 0, 15); // get hword
        set_mem(dest, value);
        break;

      case  SW:
        word base = get_reg(get_rs1(w));
        word offset = get_typeS_imm(w);
        word dest = base + offset;
        word value = get_reg(get_rs2(w)); // get word
        set_mem(dest, value);
        break;

      case  ADDI:
        break;

      case  SLTI:
        break;

      case  SLTIU:
        break;

      case  XORI:
        break;

      case  ORI:
        break;

      case  ANDI:
        break;

      case  SLLI:
        break;

      case  SRLI:
        break;

      case  SRAI:
        break;

      case  ADD:
        word dest = get_rd(w);
        word op1  = get_reg(get_rs1(w));
        word op2  = get_reg(get_rs2(w));
        word res  = op1 + op2;
        set_reg(dest, res);
        break;

      case  SUB:
        word dest = get_rd(w);
        word op1  = get_reg(get_rs1(w));
        word op2  = get_reg(get_rs2(w));
        word res  = op1 - op2;
        set_reg(dest, res);
        break;

      case  SLL:
        word dest = get_rd(w);
        word op1  = get_reg(get_rs1(w));
        word op2  = get_slice(get_reg(get_rs2(w)), 0, 4);
        word res  = op1 << op2;
        set_reg(dest, res);
        break;

      case  SLT:
        word dest = get_rd(w);
        word op1  = get_reg(get_rs1(w));
        word op2  = get_reg(get_rs2(w));
        word res  = (signed) op1 < (signed) op2 ? 1 : 0;
        set_reg(dest, res);
        break;

      case  SLTU:
        word dest = get_rd(w);
        word op1  = get_reg(get_rs1(w));
        word op2  = get_reg(get_rs2(w));
        word res  = op1 < op2 ? 1 : 0;
        set_reg(dest, res);
        break;

      case  XOR:
        word dest = get_rd(w);
        word op1  = get_reg(get_rs1(w));
        word op2  = get_reg(get_rs2(w));
        word res  = op1 ^ op2;
        set_reg(dest, res);
        break;

      case  SRL:
        word dest = get_rd(w);
        word op1  = get_reg(get_rs1(w));
        word op2  = get_slice(get_reg(get_rs2(w)), 0, 4);
        word res  = op1 >> op2;
        set_reg(dest, res);
        break;

      case  SRA:
        word dest = get_rd(w);
        word op1  = get_reg(get_rs1(w));
        word op2  = get_slice(get_reg(get_rs2(w)), 0, 4);
        word res  = op1 >> op2;
        if (op1 & (1 << 31))
        res |= (1 << 31);
        set_reg(dest, res);
        break;

      case  OR:
        word dest = get_rd(w);
        word op1  = get_reg(get_rs1(w));
        word op2  = get_reg(get_rs2(w));
        word res  = op1 | op2;
        set_reg(dest, res);
        break;

      case  AND:
        word dest = get_rd(w);
        word op1  = get_reg(get_rs1(w));
        word op2  = get_reg(get_rs2(w));
        word res  = op1 & op2;
        set_reg(dest, res);
        break;

      case  FENCE:
        break;

      case  FENCE_TSO:
        break;

      case  PAUSE:
        break;

      case  ECALL:
        break;

      case  EBREAK:
        break;

      default:
        break;
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
