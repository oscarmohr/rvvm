// register file
struct Registers {
  std::array<word, 32> x {0};

  // get member "x"s elements directly and catch malformed indices at runtime
  auto& operator[](word i) {
    if (i >= 32) die("Bad Register Index: ", i);
    return x[i];
  }

  // print register files first w registers
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
  // hashmap allows truly random access without allocating huge contiguous memory array
  std::map<word, byte> mem;

  // get member "mem"s elements directly
  auto& operator[](word i) { return mem[i]; }

  // get little endian 4 byte word mem[i]:mem[i+1]:mem[i+2]:mem[i+3]
  auto get_word(word i) {
    return (mem[i + 0] << 24)
        &  (mem[i + 1] << 16)
        &  (mem[i + 2] <<  8)
        &  (mem[i + 3] <<  0);
  }

  // get little endian 4 byte word mem[i]:mem[i+1]:mem[i+2]:mem[i+3]
  auto get_hword(word i) {
    return (mem[i + 2] <<  8)
        &  (mem[i + 3] <<  0);
  }

  auto set_word(word i, word value) {
    mem[i + 0] = get_slice(value,  0,  7);
    mem[i + 1] = get_slice(value,  8, 15);
    mem[i + 2] = get_slice(value, 16, 23);
    mem[i + 3] = get_slice(value, 24, 31);
  }

  auto set_hword(word i, word value) {
    mem[i + 0] = get_slice(value,  0,  7);
    mem[i + 1] = get_slice(value,  8, 15);
  }

  // print memory state
  auto print() {
    std::cout << "==================== memory>    ====================" << '\n';

    for (auto [key, value] : mem) {
      std::cout << "mem[" << key << "]\t= ";
      print_bits(value);
    }

    std::cout << "==================== <memory    ====================" << '\n';
  }
};
