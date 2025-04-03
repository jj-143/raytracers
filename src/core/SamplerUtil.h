#pragma once

#include "pcg_random.hpp"

namespace {

// Combining hashes
// from https://stackoverflow.com/a/50978188

template <typename T>
T xorshift(const T& n, int i) {
  return n ^ (n >> i);
}

// a hash function with another name as to not confuse with std::hash
uint64_t distribute(const uint64_t& n) {
  uint64_t p = 0x5555555555555555ull;    // pattern of alternating 0 and 1
  uint64_t c = 17316035218449499591ull;  // random uneven integer constant;
  return c * xorshift(p * xorshift(n, 32), 32);
}

template <class T>
inline size_t hashCombine(std::size_t& seed, T v) {
  return std::rotl(seed, std::numeric_limits<size_t>::digits / 3) ^
         distribute(std::hash<T>{}(v));
}

inline void hashRecursive(size_t& s) {}

template <typename T, typename... Args>
inline void hashRecursive(size_t& s, T v, Args... args) {
  s = hashCombine(s, v);
  hashRecursive(s, args...);
}

}  // namespace

/** ----------------------------------------------------------------------
 *  Sampler Utils
 */
namespace SamplerUtil {

template <typename... Args>
inline size_t hash(Args... args) {
  size_t seed = 0;
  hashRecursive(seed, args...);
  return seed;
}

}  // namespace SamplerUtil
