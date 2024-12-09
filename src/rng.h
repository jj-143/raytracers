#pragma once

#include <omp.h>

#include <random>
#include <vector>

static std::vector<std::minstd_rand> engines;

inline void initRNG(uint_fast32_t seed) {
  srand(seed + 1);
  for (int i = 0; i < omp_get_max_threads(); i++) {
    engines.push_back(std::minstd_rand(rand()));
  }
}

inline float randf() {
  std::minstd_rand& sr = engines[omp_get_thread_num()];
  return (sr() - sr.min()) / float(sr.max() - sr.min());
}

inline float randf(float low, float high) {
  return low + randf() * (high - low);
}

inline uint_fast32_t randInt() {
  std::minstd_rand& sr = engines[omp_get_thread_num()];
  return sr() - sr.min();
}
