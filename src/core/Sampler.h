#pragma once

#include <random>

#include "SamplerUtil.h"
#include "pcg_random.hpp"

namespace Sampler {

class Sampler {
 public:
  int spp;

  Sampler(int spp, int seed) : spp(spp), seed(seed) {}

  virtual void startPixelSample(Vec2i p, int index, int dim = 0) {};
  virtual float get1D() { return {}; }
  virtual Vec2f get2D() { return {}; }
  inline float uniform() { return uniformDist(rng); }

 protected:
  int seed;
  pcg32 rng;
  std::uniform_real_distribution<float> uniformDist;
};

/**
 * Uniform random values inside the pixel
 */
class RandomSampler : public Sampler {
 public:
  RandomSampler(int spp, int seed = 0) : Sampler(spp, seed) {}

  inline void startPixelSample(Vec2i p, int index, int dim = 0) override {
    rng.seed(SamplerUtil::hash(p.x, p.y, seed));
    rng.advance(index * 65536ull + dim);
  }

  inline float get1D() override { return uniform(); }

  inline Vec2f get2D() override { return {uniform(), uniform()}; }
};

/**
 * Using thread's sampler
 */

thread_local std::shared_ptr<Sampler> sampler;

inline std::shared_ptr<Sampler> InitSampler(int spp, int seed) {
  sampler = std::make_shared<RandomSampler>(spp, seed);
  return sampler;
}

inline float Get1D() { return sampler->get1D(); }
inline Vec2f Get2D() { return sampler->get2D(); }

}  // namespace Sampler
