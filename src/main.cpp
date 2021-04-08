// rvvm -- RISCV-VM
// by oscarmohr
// see ISA specifications at https://riscv.org/technical/specifications/

#include <cstdint>
#include <fstream>
#include <iostream>
#include <bitset>
#include <map>
#include <vector>
#include <array>

#include "util.hpp"
#include "instructions.hpp"
#include "memory.hpp"
#include "cpu.hpp"
#include "program.hpp"

int main(int argc, char **argv) {
  /* CPU cpu; */

  /* cpu.regs[0] = 0b1111'0000'0000'0000'0111'1111'1100'0000; */
  /* cpu.regs[1] = 0b1111'0000'0000'0000'0111'1111'1100'0000; */
  /* cpu.regs[2] = 0b1111'0000'0000'0000'0111'1111'1100'0000; */

  /* cpu.pc = 0; */

  /* cpu.mem[0] = -1; */
  /* cpu.mem[1] = -2; */
  /* cpu.mem[2] = -3; */

  /* cpu.print(); */

  Word W = Word(-1);
  word w = word(-1);

  print_bits(w);
  print_bits(W[{0, 7}]);

  print_hex(w);
  print_hex(W[{0, 7}]);

  // testing
  /* word add_instr  = 0x003100b3; */
  /* word sub_instr  = 0x403100b3; */
  /* word sll_instr  = 0x003110b3; */
  /* word slt_instr  = 0x003120b3; */
  /* word sltu_instr = 0x003130b3; */
  /* word xor_instr  = 0x003140b3; */
  /* word srl_instr  = 0x003150b3; */
  /* word sra_instr  = 0x403150b3; */
  /* word or_instr   = 0x003160b3; */
  /* word and_instr  = 0x003170b3; */ 

  /* if      (argc == 1) run_repl(); */
  /* print(argc); */
  /* if      (argc == 1) run_repl(); */
  /* else if (argc == 2) run_file(std::string(argv[1])); */
  /* else die("usage: rvvm [<filename>]"); */
}
