using namespace olib;

using word = u32;
using byte = u8;

// -------------------------------------------
inline auto slice(word w, int l, int u) {
  return (word) (w << (31 - u)) >> (31 - u + l);
}

// returns word with bits w[u:l] ones and zeroes else
inline auto slice_mask(int l, int u) {
  /* return (((word) -1) << l) >> (31 - u); */
  return ((word) slice(-1, l, u)) << l;
}

// get bits w[u:l] as least significant digits
inline auto get_slice(word w, int l, int u) {
  return (word) (w << (31 - u)) >> (31 - u + l);
}

// changes bits w[u:l] to the bits v[u-l+1:0]
// soon: w.at(l, u) = v.at(0, u - l);
inline auto set_slice(word& w, word v, int l, int u) {
  // align v with slice and zero bits not in slice
  v <<= l;
  v &= slice_mask(l, u);

  // zero bits in slice and set them to the slice in v
  w &= ~slice_mask(l, u);
  w |= v;

  return w;
}

inline auto print_bits(byte w) {
  put("0b'");
  put(std::bitset<4>(get_slice(w, 4, 7)));
  put("'");
  put(std::bitset<4>(get_slice(w, 0, 3)));
}

inline auto hex(word w) {
  put("0x'");
  put(std::hex);
  put(w);
}

inline auto print_hex(word w) {
  put("0x'");
  out << std::hex << w << '\n';
}

inline auto print_bits(word w) {
  put("0b");
  // print 8 apostroph-seperated nibbles
  for (int i = 7; i >= 0; i--) {
    put("'");
    put(std::bitset<4>(get_slice(w, i * 4, (i * 4) + 3)));
  }
  put("\n");
}

inline auto set_bits(word& w, int l, int u) { w |= slice_mask(l, u); }

inline auto bit_at(word w, int i) { return get_slice(w, i, i); }

// sign extend
inline auto sxt(word& w, int from, int to) {
  if (bit_at(w, from))
    w |=  slice_mask(from + 1, to);
  else
    w &= ~slice_mask(from + 1, to);
}

// Word structure for processing all machine words
struct Word : public word { // alternative
/* struct Word { */
  word w;

  Word()        : w{0}  {}
  Word(word _w) : w{_w} {}
  
  auto at(u32 i)                  { return get_slice(w, i, i); }
  auto at(u32 l, u32 u)           { return get_slice(w, l, u); }

  auto operator()(u32 i)          { return at(i, i); }
  auto operator()(u32 l, u32 u)   { return at(l, u); }

  auto set(word v, word l, word u) { return set_slice(w, v, l, u); }
  auto set(word v)                 { return set_slice(w, v, 0, 31); }

  auto operator=(Word r)           { w = r.w;}
};

inline auto sxt(Word w) {
  /* w & (1 << 31) ? 1:0; */
  return w;
}
