#pragma once

#include <memory>
#include <random>

#include "SamplerUtil.h"
#include "math.h"
#include "pcg_random.hpp"
#include "raytracers.h"

#ifdef GPU_RENDER
#include "CudaSampler.h"
#endif

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

class StratefiedSampler : public Sampler {
 public:
  StratefiedSampler(int xPixelSamples, int yPixelSamples, bool jitter,
                    int seed = 0)
      : Sampler(xPixelSamples * yPixelSamples, seed),
        xPixelSamples(xPixelSamples),
        yPixelSamples(yPixelSamples),
        jitter(jitter) {}

  inline void startPixelSample(Vec2i p, int index, int dim = 0) override {
    pixel = p;
    sampleIndex = index;
    dimension = dim;

    rng.seed(SamplerUtil::hash(p.x, p.y, seed));
    rng.advance(index * 65536ull + dimension);
  }

  inline float get1D() override {
    size_t hash = SamplerUtil::hash(pixel.x, pixel.y, dimension, seed);
    int stratum = SamplerUtil::permutationElement(sampleIndex, spp, hash);
    dimension++;

    float delta = jitter ? uniform() : 0.5f;
    return (stratum + delta) / spp;
  }

  inline Vec2f get2D() override {
    size_t hash = SamplerUtil::hash(pixel.x, pixel.y, dimension, seed);
    int stratum = SamplerUtil::permutationElement(sampleIndex, spp, hash);
    dimension += 2;

    int x = stratum % xPixelSamples;
    int y = stratum / xPixelSamples;

    float dx = jitter ? uniform() : 0.5f;
    float dy = jitter ? uniform() : 0.5f;

    return {(x + dx) / xPixelSamples, (y + dy) / yPixelSamples};
  }

 private:
  int xPixelSamples, yPixelSamples;
  bool jitter;
  Vec2i pixel;
  int sampleIndex = 0, dimension = 0;
};

/**
 * Using thread's sampler
 */

inline thread_local std::shared_ptr<Sampler> sampler;

inline std::shared_ptr<Sampler> InitSampler(int spp, int seed) {
  int xPixelSamples = std::sqrt(spp);
  int yPixelSamples = (spp / xPixelSamples);
  sampler = std::make_shared<StratefiedSampler>(xPixelSamples, yPixelSamples,
                                                true, seed);
  return sampler;
}

/* Sampler helpers */

#if defined(__CUDA_ARCH__) and defined(GPU_RENDER)

RT_DEVICE inline float Get1D() { return cuda_sampler::Get1D(); }
RT_DEVICE inline Vec2f Get2D() { return cuda_sampler::Get2D(); }

#else

inline float Get1D() { return sampler->get1D(); }
inline Vec2f Get2D() { return sampler->get2D(); }

#endif

}  // namespace Sampler
