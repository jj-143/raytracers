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

class DiracDeltaDistribution : public Distribution {
 public:
  DiracDeltaDistribution(Vec3f w) : w(w) {}
  Vec3f sample(const Vec3f& wo) const override { return w; }
  float pdf(const Vec3f& wo, const Vec3f& wi) const override { return 0; }

 private:
  Vec3f w;
};

/* Trowbridge–Reitz (GGX) distribution model (isotropic) */
class GGXDistribution : public Distribution {
 public:
  GGXDistribution(float alpha) : alpha(alpha) {}

  Vec3f sample(const Vec3f& wo) const override {
    Vec3f wh = sampleWh(wo);
    Vec3f wi = -wo + 2 * wh * dot(wo, wh);
    if (wo.z < 0) wi.z *= -1;
    return wi;
  }

  float pdf(const Vec3f& wo, const Vec3f& wi) const override {
    Vec3f wh = (wi + wo).normalize();
    float absCosTheta = std::fabs(wh.z);
    float DValue = ndf(wh);
    float pdfWh = DValue * absCosTheta;
    return pdfWh / (4 * dot(wo, wh));
  }

  float ndf(const Vec3f& wh) const override {
    return alpha * alpha /
           (M_PI * (std::powf((wh.z * wh.z * (alpha * alpha - 1) + 1), 2)));
  }

 private:
  float alpha;

  Vec3f sampleWh(const Vec3f& wo) const {
    // Sample angles for isotropic GGX variant
    float u[2] = {randf(), randf()};
    float beta = alpha * alpha - 1;

    float cos2theta =
        1 / (beta * beta) * ((beta + 1) / (u[0] + 1 / beta) - beta);
    cos2theta = std::fmax(0, cos2theta);
    float phi = u[1] * 2 * M_PI;

    // Map sampled angles to normal direction wh
    float cosTheta = std::sqrt(cos2theta);
    float sinTheta = std::sqrt(std::max((float)0, 1 - cos2theta));
    Vec3f wh(sinTheta * std::cos(phi), sinTheta * std::sin(phi), cosTheta);

    if (!sameHemisphere(wo, wh)) wh = -wh;
    return wh;
  }
};
