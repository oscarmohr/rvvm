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

#include "../lib/olib.hpp"

#include "rvvm_word.hpp"
#include "rvvm_instruction.hpp"
#include "rvvm_program.hpp"
#include "rvvm_memory.hpp"
#include "rvvm_cpu.hpp"
#include "rvvm_interpreter.hpp"
#include "rvvm_assembler.hpp"

int main(int argc, char **argv) {
  if (argc > 1)
    rvvm_run_files(argc, argv);
  else
    rvvm_run_shell();
}
