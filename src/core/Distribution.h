#pragma once

#include "../math.h"
#include "../rng.h"
#include "common.h"

class Distribution {
 public:
  virtual Vec3f sample(const Vec3f& wo) const { return Vec3f(1, 0, 0); }
  virtual float pdf(const Vec3f& wo, const Vec3f& wi) const { return 0; }
  virtual float ndf(const Vec3f& wh) const { return 1; }
};

class DistributionSample {
 public:
  Ray::Flag flag;

  DistributionSample() = default;
  DistributionSample(Ray::Flag flag) : flag(flag) {}
  DistributionSample(std::shared_ptr<Distribution> distrib, Ray::Flag flag)
      : distrib(distrib), flag(flag) {}

  inline Ray sample(const Vec3f& wo) const {
    return Ray(distrib->sample(wo), flag);
  }

 private:
  std::shared_ptr<Distribution> distrib;
};

class CosineDistribution : public Distribution {
 public:
  Vec3f sample(const Vec3f& wo) const override {
    float r1 = randf();
    float r2 = randf();

    float phi = 2 * M_PI * r1;
    float x = std::cos(phi) * std::sqrt(r2);
    float y = std::sin(phi) * std::sqrt(r2);
    float z = std::sqrt(1 - r2);

    Vec3f wi(x, y, z);
    if (wo.z < 0) wi.z *= -1;
    return wi;
  }

  float pdf(const Vec3f& wo, const Vec3f& wi) const override {
    return std::fmax(0.f, wi.z / M_PI);
  }
};
