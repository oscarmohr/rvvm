const String PROMPT = "rvvm>";

auto get_file(char *filename) {
  std::ifstream file;
  file.open(filename, std::ios::in);
  if (!file) die("Failed to open file ", filename);
  return file;
}

auto rvvm_run_files(int argc, char **filenames) {
  auto files = get_args(argc, filenames);
}

auto rvvm_run_file(int argc, char **filenames) {
  auto program = Program(get_file(filenames[1]));
  Cpu cpu;
  if (argc > 2)
    cpu.mem = Memory(filenames[2]);
  for (auto instruction : program);
    cpu.exec(instruction);
}

auto rvvm_run_shell() {
  for (String line; line = getline(PROMPT);)
    cpu.exec(Instruction(line));
}
