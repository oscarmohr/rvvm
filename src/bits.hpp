#ifndef BITS_HPP
#define BITS_HPP

#include "olib.hpp"

namespace rvvm {

using namespace olib;
using bad_slice = std::invalid_argument;

// slice(w, l, u) = w[u]..w[l] (as lsd's zero padded to the left)
u32 slice(u32 w, u8 l, u8 u) {
  if (u < l)
    throw bad_slice("bad slice: upper index < lower index");
  if (l < 0 || u > 31)
    throw bad_slice("bad slice: index out of bounds");

  auto lshamt = 31 - u;
  auto rshamt = lshamt + l;

  return w << lshamt >> rshamt;
}

// bitmask with ones from bits l to u, zeroes elsewhere
u32 ones(u8 l, u8 u) {return slice(-1, l, u) << l;} 

// set bits l to u of w to the u - l + 1 lsd's of v
u32 set_slice(u32& w, u32 to, u8 l, u8 u) {
  to <<= l;
  to &= ones(l, u);
  w &= ~ones(l, u);
  return w |= to;
}

bool bit_at(u32 w, u8 i) {return slice(w, i, i);}

// sign extend bit from position i
u32 sxt(u32 w, u8 i) {
  auto to = bit_at(w, i) ? -1 : 0;
  if (i > 31)
    throw bad_slice("bad slice: sxt with out of bounds index");
  if (i == 31)
    return w;
  auto l = i + 1;
  auto u = 31;
  return set_slice(w, to, l, u);
}

// printing words
string bin(u32 w) {return "0b" + bitset<32>{w}.to_string();}
string bin(u8 w)  {return "0b" + bitset<8> {w}.to_string();}
string hex(u8 w) {
  auto ss = stringstream();
  ss << std::setfill('0')
     << std::setw(2)
     << std::hex
     << slice(w, 0, 7);
  return ss.str();
}
string hex(u32 w) {
  auto ss = stringstream();
  ss << "0x";
  for (auto i : range(3, 0)) {
    LOG(i);
    ss << hex((u8) slice(w, i * 8, i * 8 + 7));
  }
  return ss.str();
}
string hex_pretty(u32 w) {
  auto ss = stringstream();
  ss << hex((u8) slice(w, 24, 31)) << ' '
     << hex((u8) slice(w, 16, 23)) << ' '
     << hex((u8) slice(w,  8, 15)) << ' '
     << hex((u8) slice(w,  0,  7));
  return ss.str();
}

} // namespace rvvm

#endif // #define BITS_HPP
