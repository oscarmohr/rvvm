using word = uint32_t;
using byte = uint8_t;

// get bits w[u:l] as least significant digits
inline auto get_slice(word w, int l, int u) { return (w << (31 - u)) >> (31 - u + l); }

// might start using Word structure to encapsulate words with slice access
struct Word {
  word w;

  Word(word _w) { w = _w; }
  
  auto operator[](byte i) {
  }
  auto operator[](std::pair<byte, byte> slice) {
    return get_slice(w, slice.first, slice.second);
  }

};

// premature program exit strategy will soon not halt machine
auto die(std::string s) {
  std::cout << s;
  exit(EXIT_FAILURE);
}

template<typename T>
auto die(std::string s, T t) {
  std::cout << s << t; exit(EXIT_FAILURE);
}

template <typename T>
inline auto print(T a) { std::cout << a << '\n'; }

template <typename T>
inline auto put(T a) { std::cout << a; }

// returns word with bits w[u:l] ones and zeroes else
inline auto slice_mask(int l, int u) {
  return (((word) -1) << l) >> (31 - u);
}

// changes bits w[u:l] to the bits v[u-l+1:0]
inline auto set_slice(word& w, word v, int l, int u) {
  w &= ~slice_mask(l, u);
  v <<= l;
  v &= slice_mask(l, u);
  w |= v;
  return w;
}

inline auto set_bits(word& w, int l, int u) { w |= slice_mask(l, u); }

inline auto print_bits(byte w) {
  std::cout << "0b";
  std::cout << '\'' << std::bitset<8>(w);
  std::cout << '\n';
}

inline auto print_bits(word w) {
  std::cout << "0b";
  // print 8 apostroph-seperated nibbles
  for (int i = 7; i >= 0; i--)
    std::cout << '\'' << std::bitset<4>(get_slice(w, i*4 , i*4 + 3));
  std::cout << '\n';
}

inline auto bit_at(word w, int pos) { return get_slice(w, pos, pos); }

// sign extend
inline auto sxt(word& w, int from, int to) {
  if (bit_at(w, from)) w |=  slice_mask(from + 1, to);
  else                 w &= ~slice_mask(from + 1, to);
}
