# rvvm -- RISC-V Virtual Machine

by oscarmohr

see [ISA specifications](https://riscv.org/technical/specifications/)

Implements the rv32i base isa.


## Usage

include rvvm in your `C++` project, and instantiate a Machine like this:

```c++
...
rvvm::Machine machine;
...
```

or open up the `rvvm_shell` in a terminal to interactively instruct the virtual machine or execute meta instructions.
