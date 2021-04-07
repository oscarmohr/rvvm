struct Registers {
  std::array<word, 32> x {0};

  auto& operator[](word i) {
    if (i >= 32) die("Bad Register Index: ", i);
    return x[i];
  }

  auto print(word w) {
    std::cout << "==================== registers> ====================" << '\n';

    for (word i = 0; i < w; i++) {
      std::cout << "x" << i << "\t= ";
      print_bits(operator[](i));
    }

    std::cout << "==================== <registers ====================" << '\n';
  }
};

struct Memory {
  std::map<word, byte> mem;

  auto& operator[](word i) { return mem[i]; }

  auto get(word i) { return operator[](i); }

  auto set(word i, word value) {
    mem[i] = value;
  }

  auto get_word(word i) {
    // little endian
    word byte0 = mem[i + 0] << 24;
    word byte1 = mem[i + 1] << 16;
    word byte2 = mem[i + 2] <<  8;
    word byte3 = mem[i + 3] <<  0;
    return byte0 & byte1 & byte2 & byte3;
  }

  auto set_word(word memcell, word value) {
    if (memcell >= 32) die("Invalid memcellister index: ", memcell);
    mem[memcell] = (memcell ? value : 0);
  }

  auto print() {
    std::cout << "==================== memory>    ====================" << '\n';

    for (auto [key, value] : mem) {
      std::cout << "mem[" << key << "]\t= ";
      print_bits(value);
    }

    std::cout << "==================== <memory    ====================" << '\n';
  }
};
