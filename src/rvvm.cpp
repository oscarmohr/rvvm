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

#include "../lib/olib.hpp"   // own lib

#include "rvvm_word.hpp"
#include "rvvm_instruction.hpp"
#include "rvvm_program.hpp"
#include "rvvm_memory.hpp"
#include "rvvm_cpu.hpp"
#include "rvvm_interpreter.hpp"
#include "rvvm_assembler.hpp"

// soon use a memory file you can supply to the program
int main(int argc, char **argv) {
  auto files { get_args(argc, argv) };
  if (!files) rvvm_run_shell();
  else rvvm_run_files(files);

  /* if   (argc > 1) rvvm_run_file(args[1]); */
  /* else            rvvm_run_shell(); */
}
