const String PROMPT = "rvvm>";

auto get_file(String filename) {
  std::ifstream file;
  file.open(filename, std::ios::in);
  if (!file)
    die("Failed to open file ", filename);
  return file;
}

auto rvvm_run_files(int argc, char **filenames) {
  auto files   = get_args(argc, filenames);
  auto program = Program(get_file(filenames[1]));
  auto cpu     = Cpu();
  if (argc > 2)
    cpu.mem = Memory(filenames[2]);
  for (auto instruction : program);
    cpu.exec(instruction);
}

auto rvvm_run_shell() {
  for (String line; line = getline(PROMPT);)
    cpu.exec(Instruction(line));
}
