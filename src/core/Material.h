#pragma once

#include <array>
#include <memory>
#include <optional>

#include "../math.h"
#include "Distribution.h"
#include "common.h"

enum class MaterialType { BSDF, Emission, Phong };

class Material {
 public:
  MaterialType type;
  Vec3f albedo;
  std::shared_ptr<Distribution> distrib;

  Material(MaterialType type, Vec3f albedo) : type(type), albedo(albedo) {}

  virtual inline Vec3f f(const Ray& ro, const Ray& ri) { return 0; }

  virtual DistributionSample sampleDistribution(const Ray& ro) { return {}; }

  virtual std::optional<BSDFSample> sample(const Ray& ro, const Ray& ri) {
    return {};
  }
};

class Emission : public Material {
 public:
  Emission(Vec3f color, float strength = 1)
      : Material(MaterialType::Emission, color * strength) {}
};

class PhongMaterial : public Material {
 public:
  std::array<float, 3> constants;  // k_specular, k_reflection, k_transmission
  float specularExponent;
  float refractiveIndex;

  PhongMaterial(Vec3f albedo = Vec3f(1, 0, 0),
                std::array<float, 3> constants = {0, 0, 0},
                float specularExponent = 1, float refractiveIndex = 1)
      : Material(MaterialType::Phong, albedo),
        constants(constants),
        specularExponent(specularExponent),
        refractiveIndex(refractiveIndex) {}
};

class LambertBSDF : public Material {
 public:
  LambertBSDF(Vec3f albedo) : Material(MaterialType::BSDF, albedo) {
    distrib = std::make_shared<CosineDistribution>();
  }

  inline Vec3f f(const Ray& ro, const Ray& ri) override {
    return albedo / M_PI;
  }

  DistributionSample sampleDistribution(const Ray& ro) override {
    return DistributionSample(distrib, Ray::Diffuse);
  }

  std::optional<BSDFSample> sample(const Ray& ro, const Ray& ri) override {
    return BSDFSample(f(ro, ri), distrib->pdf(ro, ri));
  }
};
