#ifndef VM_HPP
#define VM_HPP

#include "olib.hpp"
#include "bits.hpp"
#include "rv.hpp"

namespace rvvm {

using namespace olib;

struct Machine {
  map<u32, u8> mem;
  vector<u32> regs = vector<u32>(33);
  u32 pc;

  u32& operator[](u8 i); // memory
  u32& x(u8 i); // registers

  // load/store byte/halfword/word
  u32 load(u8 n, u32 i);
  u32 lb(u32 i) {return load(1,i);}
  u32 lh(u32 i) {return load(2,i);}
  u32 lw(u32 i) {return load(4,i);}

  void store(u8 n, u32 w, u32 i);
  void sb(u32 w, u32 i) {store(1, w, i);}
  void sh(u32 w, u32 i) {store(2, w, i);}
  void sw(u32 w, u32 i) {store(4, w, i);}

  Machine() {};

  string str();

  void operator()(u32 i);
  void operator()(){operator()(lw(pc));}
};

// load 0 <= n <= 4 bytes starting from adress i into word
// TODO: misaligned loads should throw
u32 Machine::load(u8 n, u32 i) {
  if (n > 2) n = 4; // only load 1, 2 or 4 bytes

  u32 w;
  for (auto j = 0; j < n; j++) {
    auto l = j * 8;
    auto u = l + 7;
    auto to = mem.contains(i + j) ? mem[i + j] : 0;
    set_slice(w, to, l, u);
  }

  // sxt
  return w;
}

// TODO: misaligned stores should throw
void Machine::store(u8 n, u32 w, u32 i) {
  if (n > 2) n = 4; // only store 1, 2 or 4 bytes

  for (auto j = 0; j < n; j++) {
    auto l = j * 8;
    auto u = l + 7;
    mem[i + j] = slice(w, l, u);
  }
}


string Machine::str() {
  const string header  = "┌─────────────────────────────┐\n"
                         "│       Registers             │\n"
                         "├──┬───────────┬──┬───────────┤\n";
                       // │ 0│00 00 00 00│16│00 00 00 00│
                       //               ...              
  const string middle =  "├──┴───────────┴──┴───────────┤\n"
                         "│        Memory               │\n"
                         "├───────────┬───┬────┬────┬───┤\n";
                       // │00 00 00 00│93 │ .. │ 10 │ 00│
  const string footer =  "└───────────┴───┴────┴────┴───┘\n";

  stringstream ss;
  ss << header;

  auto print_registers = [&]{
    auto register_line = [&](auto i) {
      stringstream ss;
      ss << "│" << std::setw(2) << std::setfill(' ')
         << i      << "│" << hex_pretty(regs[i])
         << "│" << std::setw(2) << std::setfill(' ')
         << i + 16 << "│" << hex_pretty(regs[i + 16]) << "│\n";
      return ss.str();
    };

    for (auto i : range_(16))
      ss << register_line(i);
  };

  // holds the word aligned (divisible by 4) byte adress 'memory_line_index'
  // such that memory_line_index <= index <= memory_line_index + 3
  auto print_memory = [&]{
    auto memory_line_index      = 0;
    auto prev_memory_line_index = 0;
    auto empty_line = "\n              ...             \n\n";
    auto byte = [&](auto i) { return mem.contains(i) ? hex(mem[i]) : ".."; };
    auto memory_line = [&](auto i) {
      stringstream ss;
      ss << "│"   << hex_pretty(memory_line_index)
         << "│"   << byte(i)
         << " │ " << byte(i + 1)
         << " │ " << byte(i + 2)
         << " │ " << byte(i + 3) << "│\n";
      return ss.str();
    };

    for (const auto [index, val] : mem) {
      // this index has been printed in previous memory line:
      if (0 < index && index <= memory_line_index + 3) continue;
      // get the current memory cells memory line index:
      memory_line_index = index - (index % 4);
      // if the adjacent difference is greater than 4, we skipped a line:
      if (memory_line_index - prev_memory_line_index > 4) ss << empty_line;
      ss << memory_line(index);
      prev_memory_line_index = memory_line_index;
    }
  };

  print_registers();
  ss << middle;
  print_memory();
  ss << footer;
  return ss.str();
}

void Machine::operator()(u32 i) {
  auto instr = Instruction(i);
  auto& rd = regs[instr.rd];
  auto rs1 = regs[instr.rs1];
  auto rs2 = regs[instr.rs2];
  auto imm = instr.imm;
  auto inc_pc = [&]{pc+=4;};

  auto exec_lui = [&]{ rd = imm; inc_pc(); };
  auto exec_auipc = [&]{ pc += imm; inc_pc();};

  // store successor instruction address into rd
  // add sext J imm (offset) to pc
  auto exec_jal  = [&]{ rd = pc + 4; pc += imm; };

  // store successor instruction address into rd
  // add sext I imm to rs1 and set lsd of the result to zero
  auto exec_jalr = [&]{ rd = pc + 4; pc = (rs1 + imm) & ones(31, 1); };

  auto exec_beq  = [&]{ pc += (rs1 == rs2) ? imm : 4;};
  auto exec_bne  = [&]{ pc += (rs1 != rs2) ? imm : 4;};
  auto exec_blt  = [&]{ pc += (rs1  < rs2) ? imm : 4;};
  auto exec_bge  = [&]{ pc += (rs1 >= rs2) ? imm : 4;};
  auto exec_bltu = [&]{ pc += (rs1  < rs2) ? imm : 4;};
  auto exec_bgeu = [&]{ pc += (rs1 >= rs2) ? imm : 4;};

  // load and store // TODO: sxt and not
  auto exec_lb  = [&]{ rd = lb(rs1 + imm); inc_pc();};
  auto exec_lh  = [&]{ rd = lh(rs1 + imm); inc_pc();};
  auto exec_lw  = [&]{ rd = lw(rs1 + imm); inc_pc();};
  auto exec_lbu = [&]{ rd = lb(rs1 + imm); inc_pc();};
  auto exec_lhu = [&]{ rd = lh(rs1 + imm); inc_pc();};
  auto exec_sb  = [&]{ sb(rs1 + imm, slice(rs2, 0,  7)); inc_pc();};
  auto exec_sh  = [&]{ sh(rs1 + imm, slice(rs2, 0, 15)); inc_pc();};
  auto exec_sw  = [&]{ sw(rs1 + imm, rs2);               inc_pc();};

  auto exec_addi  = [&]{rd = rs1  + imm;              inc_pc();};
  auto exec_slti  = [&]{rd = rs1  < imm;              inc_pc();};
  auto exec_sltiu = [&]{rd = rs1  < imm;              inc_pc();};
  auto exec_xori  = [&]{rd = rs1  ^ imm;              inc_pc();};
  auto exec_ori   = [&]{rd = rs1  | imm;              inc_pc();};
  auto exec_andi  = [&]{rd = rs1  & imm;              inc_pc();};
  auto exec_slli  = [&]{rd = rs1 << imm;              inc_pc();};
  auto exec_srli  = [&]{rd = rs1 >> imm;              inc_pc();};
  auto exec_srai  = [&]{rd = rs1 >> imm;              inc_pc();};
  auto exec_add   = [&]{rd = rs1  + rs2;              inc_pc();};
  auto exec_sub   = [&]{rd = rs1  - rs2;              inc_pc();};
  auto exec_sll   = [&]{rd = rs1 << slice(rs2, 0, 4); inc_pc();};
  auto exec_slt   = [&]{rd = int(rs1) < int(rs2);     inc_pc();};
  auto exec_sltu  = [&]{rd = rs1  < rs2;              inc_pc();};
  auto exec_xor   = [&]{rd = rs1  ^ rs2;              inc_pc();};
  auto exec_srl   = [&]{rd = rs1 >> slice(rs2, 0, 4); inc_pc();};
  auto exec_sra   = [&]{rd = rs1 >> slice(rs2, 0, 4); inc_pc();};
  auto exec_or    = [&]{rd = rs1  | rs2;              inc_pc();};
  auto exec_and   = [&]{rd = rs1  & rs2;              inc_pc();};

  auto exec_fence     = [&]{inc_pc();};
  auto exec_fence_tso = [&]{inc_pc();};
  auto exec_ecall     = [&]{inc_pc();};
  auto exec_ebreak    = [&]{inc_pc();};

  switch (instr.instr) {
  using enum rvInstruction;
  case LUI:       return exec_lui();         
  case AUIPC:     return exec_auipc();         
  case JAL:       return exec_jal();         
  case JALR:      return exec_jalr();         
  case BEQ:       return exec_beq();         
  case BNE:       return exec_bne();         
  case BLT:       return exec_blt();         
  case BGE:       return exec_bge();         
  case BLTU:      return exec_bltu();         
  case BGEU:      return exec_bgeu();         
  case LB:        return exec_lb();         
  case LH:        return exec_lh();         
  case LW:        return exec_lw();         
  case LBU:       return exec_lbu();         
  case LHU:       return exec_lhu();         
  case SB:        return exec_sb();         
  case SH:        return exec_sh();         
  case SW:        return exec_sw();         
  case ADDI:      return exec_addi();         
  case SLTI:      return exec_slti();         
  case SLTIU:     return exec_sltiu();         
  case XORI:      return exec_xori();         
  case ORI:       return exec_ori();         
  case ANDI:      return exec_andi();         
  case SLLI:      return exec_slli();         
  case SRLI:      return exec_srli();         
  case SRAI:      return exec_srai();         
  case ADD:       return exec_add();         
  case SUB:       return exec_sub();         
  case SLL:       return exec_sll();         
  case SLT:       return exec_slt();         
  case SLTU:      return exec_sltu();         
  case XOR:       return exec_xor();         
  case SRL:       return exec_srl();         
  case SRA:       return exec_sra();         
  case OR:        return exec_or();         
  case AND:       return exec_and();         
  case FENCE:     return exec_fence();         
  case FENCE_TSO: return exec_fence_tso();         
  case ECALL:     return exec_ecall();         
  case EBREAK:    return exec_ebreak();         
  }
}

} // namespace rvvm

#endif // #define VM_HPP
