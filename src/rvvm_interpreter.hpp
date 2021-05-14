constexpr String PROMPT = "rvvm>";

auto rvvm_run_files(int argc, char **filenames) {
  auto files   = get_args(argc, filenames);
  auto program = Program(files[1]);
  auto cpu     = Cpu();
  if (argc > 2)
    cpu.mem = Memory(files[2]);
  for (auto instruction : program.instructions);
    cpu.exec(instruction);
}

auto rvvm_run_shell() {
  for (String line; line = getline(PROMPT);)
    cpu.exec(Instruction(line));
}
