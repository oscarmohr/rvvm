// register file
struct Registers {
  /* std::map<RegName, std::string> r; */
  std::array<word, 32> x {0};

  // access elements directly, catch malformed indices at runtime
  // TODO do not return reference to zero register (maybe dummy register)
  auto& operator[](u32 i) {
    if (i >= 32) olib::die("Bad Register Index: ", i);
    return x[i];
  }

  // print register files first count registers
  auto print(u32 count) {
    olib::print("==================== registers> ====================");

    for (auto i : range(count)) {
      /* putf("x{i}\t= {operator[](i).w.bits()}" */
      put("x"); put(i); put("\t= ");
      print_bits(operator[](i));
    }

    olib::print("==================== <registers ====================");
  }
};

struct Memory {
  // map for random access without huge contiguous array
  Map<word, byte> mem;

  // get member "mem"s elements directly
  inline auto& operator[](word i) { return mem[i]; }
  inline auto& at(word i) { return operator[](i); }
  inline auto  set(word i) { return operator[](i); }
  inline auto& byte_at(word i);
  inline auto& hword_at(word i);
  inline auto& word_at(word i);

  // get little endian 4 byte word mem[i]:mem[i+1]:mem[i+2]:mem[i+3]
  inline auto get_word(word i) {
    return (mem[i + 0] <<  0)
        &  (mem[i + 1] <<  8)
        &  (mem[i + 2] << 16)
        &  (mem[i + 3] << 24);
  }

  // get little endian 16 halfword
  inline auto get_hword(word i) {
    return (mem[i + 0] <<  0)
        &  (mem[i + 1] <<  8);
  }

  inline auto get_byte(word i) {
    return mem[i];
  }

  inline auto set_word(word i, word value) {
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
    olib::print("==================== memory>    ====================");

    for (auto [key, value] : mem) {
      put("mem["); put(key); put("]\t= ");
      print_bits(value);
      put('\n');
    }

    olib::print("==================== <memory    ====================");
  }

  // construct memory from memoryfile
  Memory(String filename) {
  }
};
