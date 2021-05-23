#include <cstdint>
#include <fstream>
#include <istream>
#include <iostream>
#include <map>
#include <vector>
#include <numeric>
#include <bitset>

namespace olib {

////////////////////////////////////////////////////////////
// types
////////////////////////////////////////////////////////////

using i32 = int32_t;
using i64 = int64_t;
using u32 = uint32_t;
using u64 = uint64_t;
using u8  = uint8_t;

using String = std::string;

template <typename T>
using Vector = std::vector<T>;

template <typename T1, typename T2>
using Map    = std::map<T1, T2>;

auto& in = std::cin;
auto& out = std::cout;


////////////////////////////////////////////////////////////
// Input/Output
////////////////////////////////////////////////////////////

// as recursion base
auto print() { std::cout << std::endl; }

// variadic version
template <typename T, typename... Ts> 
auto print(T t, Ts... ts) {
  std::cout << t;
  print(ts...);
}
auto put() { } // as recursion base

template <typename T, typename... Ts> // variadic version
auto put(T t, Ts... ts) {
  std::cout << t;
  put(ts...);
}

template<typename T>
auto print_vector(Vector<T> vector) {
  put("(");
  for (auto element : vector)
    put(element, " ");
  print(")");
}

template<typename T>
auto print_templated(T printee) {
  for (auto element : printee)
    out << element;
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

template <typename T>
auto output(T t) {
  put(t);
}

auto input(String s) {
  put(s);
  in >> s;
  return s;
}

auto die(String s) {
  print(s);
  exit(EXIT_FAILURE);
}

template <typename T>
auto die(String s, T t) {
  put(s); put(t);
  exit(EXIT_FAILURE);
}

template <typename T>
auto inform(String s, T t) {
  put(s);
  print(t);
}

auto get_file(String filename) {
  std::ifstream file;
  file.open(filename, std::ios::in);
  if (!file)
    die("Failed to open file ", filename);
  return file;
}


////////////////////////////////////////////////////////////
// Ranges
////////////////////////////////////////////////////////////

template<typename T>
auto reverse(T t) {
  std::reverse(t.begin(), t.end());
}

template<typename T>
inline auto min(T t1, T t2) {
  return t1  <= t2 ? t1 : t2;
}

template<typename T>
inline auto max(T t1, T t2) {
  return t1  <= t2 ? t2 : t1;
}

// fill container ascendingly from from
template<typename T>
auto fill(T& t, int from) {
  std::iota(t.begin(), t.end(), from);
}

// inclusive discrete ranges [ from, ..., to ]
Vector range(int from, int to) {
  auto minimum = min(from, to);
  auto maximum = max(from, to);
  auto size    = maximum - minimum + 1;
  auto vector  = Vector<int>(size);

  fill(vector, minimum);
  if (from != minimum) reverse(vector);
  return vector;
}

// python type range [ 0, ..., to )
auto range(int to) {
  if (0 < to) return range(0, to - 1);
  if (0 > to) return range(0, to + 1);
  return Vector<int>(); // if from == to
}


////////////////////////////////////////////////////////////
// arguments
////////////////////////////////////////////////////////////

auto get_args(int argc, char **argv) {
  auto args = Vector<String>();
  for (auto i : range(0, argc - 1))
    args.push_back(String(argv[i]));
  return args;
}

}; // namespace olib
