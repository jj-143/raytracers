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

/**
 * From pbrt
 */
inline int permutationElement(uint32_t i, uint32_t l, uint32_t p) {
  uint32_t w = l - 1;
  w |= w >> 1;
  w |= w >> 2;
  w |= w >> 4;
  w |= w >> 8;
  w |= w >> 16;
  do {
    i ^= p;
    i *= 0xe170893d;
    i ^= p >> 16;
    i ^= (i & w) >> 4;
    i ^= p >> 8;
    i *= 0x0929eb3f;
    i ^= p >> 23;
    i ^= (i & w) >> 1;
    i *= 1 | p >> 27;
    i *= 0x6935fa69;
    i ^= (i & w) >> 11;
    i *= 0x74dcb303;
    i ^= (i & w) >> 2;
    i *= 0x9e501cc3;
    i ^= (i & w) >> 2;
    i *= 0xc860a3df;
    i &= w;
    i ^= i >> 5;
  } while (i >= l);
  return (i + p) % l;
}

}  // namespace SamplerUtil
