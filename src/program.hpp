struct Program {
  std::vector<word> instructions;
  std::vector<byte> data_segment;

  auto add_instr(word w) {
    instructions.push_back(w);
  }
  auto add_data(byte w) {
    data_segment.push_back(w);
  }
};
