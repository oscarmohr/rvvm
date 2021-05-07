/*                                               
    ____
   / / /   _ ____   ____   ___ __ ___
  / / /   | '__\ \ / /\ \ / / '_ ` _ \
 / / /    | |   \ V /  \ V /| | | | | |
/_/_/     |_|    \_/    \_/ |_| |_| |_|


          rvvm -- RISCV-VM by @oscarmohr
          see github.com/oscarmohr/rvvm
          see riscv.org/technical/specifications
                                                  */

#include "/home/o/olib.hpp"   // own lib

#include "rvvm_word.hpp"
#include "rvvm_instruction.hpp"
#include "rvvm_program.hpp"
#include "rvvm_memory.hpp"
#include "rvvm_cpu.hpp"
#include "rvvm_interpreter.hpp"
#include "rvvm_assembler.hpp"

auto get_args(int argc, char **argv) {
  auto args = std::vector<std::string>;
  for (auto i : range(argc - 1))
    args.push_back(std::string(argv[i]));
  return args;
}

// soon use a memory file you can supply to the program
int main(int argc, char **argv) {
  /* auto args = get_args(argc, argv); */
  /* if (!args) */
  /*   rvvm_run_shell(); */
  /* else */
  /*   for (auto file : args) */
  /*     rvvm_run_file(file); */

  if   (argc > 1) rvvm_run_file(args[1]);
  else            rvvm_run_shell();
}
