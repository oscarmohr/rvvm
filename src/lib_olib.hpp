#include "lib_stdlib.hpp"

namespace olib {

using i32 = int32_t;
using u32 = uint32_t;
using u8  = uint8_t;
using String = std::string;

template <typename T>
using Vector = std::vector<T>;

template <typename T1, typename T2>
using Map    = std::map<T1, T2>;

auto& in = std::cin;
auto& out = std::cout;

template <typename T>
auto print(T t) {
  std::cout << t << std::endl;
}

template <typename T>
auto put(T t) {
  std::cout << t;
}

template<typename T>
auto print_vector(Vector<T> v) {
  for (auto elem : v)
    print(elem);
  print("");
}

auto getline() {
  String line;
  std::getline(in, line);
  return line;
}

template <typename T>
auto getline(T t) {
  put(t);
  return getline();
}

template <typename T>
auto get() {
  T t;
  in >> t;
  return t;
}

auto die(String s) {
  print(s);
  exit(EXIT_FAILURE);
}

template <typename T>
auto die(String s, T t) {
  put(s);
  print(t);
  exit(EXIT_FAILURE);
}


//
// Ranges
//

template<typename T>
auto reverse(T t) {
  std::reverse(t.begin(), t.end());
}

auto _rng(int l, int u) {
  auto min { l < u ? l : u };
  auto max { l < u ? u : l };

  auto size { max - min + 1 };
  Vector<int> r(size);

  std::iota(r.begin(), r.end(), min);

  if (l > u)
    reverse(r);

  return r;
}

auto range(int l, int u) {
  return _rng(l, u);
}

auto range(int n) {
  return n >= 0 ?  range(0, n) : range(n, 0);
}

template<typename T>
auto fill(T t, int from) {
  std::iota(t.begin(), t.end(), from);
}

template<typename T>
inline auto min(T t1, T t2) {
  return t1  <= t2 ? t1 : t2;
}

template<typename T>
inline auto max(T t1, T t2) {
  return t1  <= t2 ? t2 : t1;
}

auto rng(int l, int u) {
  /* auto ll   { l < u ? l : u }; */
  /* auto uu   { l < u ? u : l }; */
  auto ll     = min(l, u);
  auto uu     = max(l, u);
  auto size   = uu - ll;
  auto r      = Vector<int>(size);
  /* Vector<int> r(size); */
  auto from   = ll;
  fill(r, from);
  if (l > u)
    reverse(r);
  return r;
}

auto rng(int u) {
  return rng(0, u);
}

/* // numeric ranges as iterable containers */
/* auto range(u32 l, u32 u) { */
/*   std::vector<u32> _range(u - l + 1); */
/*   std::iota(_range.begin(), _range.end(), l); */
/*   return _range; */
/* } */
/* auto range(u32 u) { return range(0, u); } */

}; // namespace olib
