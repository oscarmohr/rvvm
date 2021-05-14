struct Program {
  auto instructions = Vector<Instruction>();
  auto data_segment = Vector<byte>();

  auto add_instr(Instruction w) { instructions.push_back(w); }
  auto add_data(byte w)         { data_segment.push_back(w); }

  Program(String filename) {
    std::ifstream file;
    file.open(filename, std::ios::in);

    if (!file)
      die("Failed to open file ", filename);

    auto lines = Vector<String>();

    for (auto line : lines)
      add_instr(Instruction(line));
  }

  Program(char **filename_chars) {
    auto filename = (String) filename_chars[1];
    Program(filename);
  }
};
