#pragma once

#include "Sampler.h"
#include "math.h"

class CosineDistribution {
 public:
  Vec3f sample(const Vec3f& wo) const {
    Vec2f u = Sampler::Get2D();

    float phi = 2 * M_PI * u.x;
    float x = std::cos(phi) * std::sqrt(u.y);
    float y = std::sin(phi) * std::sqrt(u.y);
    float z = std::sqrt(1 - u.y);

    Vec3f wi(x, y, z);
    if (wo.z < 0) wi.z *= -1;
    return wi;
  }

  float pdf(const Vec3f& wo, const Vec3f& wi) const {
    return std::fmax(0.f, wi.z / M_PI);
  }
};

class DiracDeltaDistribution {
 public:
  DiracDeltaDistribution(Vec3f w) : w(w) {}

  Vec3f sample(const Vec3f& wo) const { return w; }
  float pdf(const Vec3f& wo, const Vec3f& wi) const { return 0; }

 private:
  Vec3f w;
};

/* Trowbridge–Reitz (GGX) distribution model (isotropic) */
class GGXDistribution {
 public:
  GGXDistribution(float alpha) : alpha(alpha) {}

  Vec3f sample(const Vec3f& wo) const {
    Vec3f wh = sampleWh(wo);
    Vec3f wi = -wo + 2 * wh * dot(wo, wh);
    if (wo.z < 0) wi.z *= -1;
    return wi;
  }

  float pdf(const Vec3f& wo, const Vec3f& wi) const {
    Vec3f wh = (wi + wo).normalize();
    float absCosTheta = std::fabs(wh.z);
    float DValue = ndf(wh);
    float pdfWh = DValue * absCosTheta;
    return pdfWh / (4 * dot(wo, wh));
  }

  float ndf(const Vec3f& wh) const {
    return alpha * alpha /
           (M_PI * (std::powf((wh.z * wh.z * (alpha * alpha - 1) + 1), 2)));
  }

 private:
  float alpha;

  Vec3f sampleWh(const Vec3f& wo) const {
    // Sample angles for isotropic GGX variant
    Vec2f u = Sampler::Get2D();
    float beta = alpha * alpha - 1;

    float cos2theta =
        1 / (beta * beta) * ((beta + 1) / (u.x + 1 / beta) - beta);
    cos2theta = std::fmax(0, cos2theta);
    float phi = u.y * 2 * M_PI;

    // Map sampled angles to normal direction wh
    float cosTheta = std::sqrt(cos2theta);
    float sinTheta = std::sqrt(std::max((float)0, 1 - cos2theta));
    Vec3f wh(sinTheta * std::cos(phi), sinTheta * std::sin(phi), cosTheta);

    if (!sameHemisphere(wo, wh)) wh = -wh;
    return wh;
  }
};
