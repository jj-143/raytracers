#pragma once

#include <array>
#include <memory>
#include <optional>

#include "../math.h"
#include "../rng.h"
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

class MetalBSDF : public Material {
 public:
  MetalBSDF(Vec3f albedo) : Material(MaterialType::BSDF, albedo) {}

  inline Vec3f f(const Ray& ro, const Ray& ri) override {
    return IsSpecular(ri.flag) ? albedo : 0;
  }

  DistributionSample sampleDistribution(const Ray& ro) override {
    Vec3f reflected;
    reflect(ro, reflected);
    return DistributionSample(
        std::make_shared<DiracDeltaDistribution>(reflected), Ray::Specular);
  }

  std::optional<BSDFSample> sample(const Ray& ro, const Ray& ri) override {
    return BSDFSample(f(ro, ri), 0);
  }
};

class DielectricBSDF : public Material {
 public:
  float ior;  // Like in Blender, it's a relative value to surroundings.
  float R0;   // Reflectance at wi = 0

  DielectricBSDF(float ior) : Material(MaterialType::BSDF, Vec3f(1)), ior(ior) {
    R0 = std::powf((ior - 1) / (ior + 1), 2);
  }

  inline Vec3f f(const Ray& ro, const Ray& ri) override {
    return IsSpecular(ri.flag) ? 1 : 0;
  }

  DistributionSample sampleDistribution(const Ray& ro) override {
    Ray ri;
    float VoH = ro.z;
    bool isInside = VoH < 0;
    float eta = isInside ? ior : 1.f / ior;
    float R = FresnelSchlick(std::abs(VoH), R0);
    Vec3f N = isInside ? Vec3f(0, 0, -1) : Vec3f(0, 0, 1);

    bool isAllowedAngle = refract(-ro, N, eta, ri);
    bool isTransmission = isAllowedAngle && randf() < (1 - R);

    if (isTransmission) {
      ri.normalize();
    } else {
      reflect(ro, ri);
    }

    return DistributionSample(
        std::make_shared<DiracDeltaDistribution>(ri),
        isTransmission ? Ray::SpecularTransmission : Ray::SpecularReflection);
  }

  std::optional<BSDFSample> sample(const Ray& ro, const Ray& ri) override {
    return BSDFSample(f(ro, ri), 0);
  }
};
