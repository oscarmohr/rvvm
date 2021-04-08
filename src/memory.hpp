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
    word byte3 = mem[i + 0] << 24;
    word byte2 = mem[i + 1] << 16;
    word byte1 = mem[i + 2] <<  8;
    word byte0 = mem[i + 3] <<  0;
    return byte3 & byte2 & byte1 & byte0;
  }

  auto set_word(word memcell, word value) {
    if (memcell >= 32) die("Invalid memcellister index: ", memcell);
    mem[memcell] = (memcell ? value : 0);
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
