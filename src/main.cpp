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
  if      (argc == 1) run_repl();
  else if (argc == 2) run_file(std::string(argv[1]));
  else                die("usage: rvvm [<filename>]");
}
