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

  Material(MaterialType type) : type(type) {}
  Material(MaterialType type, Vec3f albedo) : type(type), albedo(albedo) {}

  virtual float E(const Vec3f& wo) const { return 0; }

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

class Microfacet : public Material {
 public:
  float roughness;
  float alpha;
  float ior;  // see: `DielectricBSDF`
  float R0;   // Reflectance at wi = 0

  Microfacet(Vec3f albedo, float roughness, float ior)
      : Material(MaterialType::BSDF, albedo), roughness(roughness), ior(ior) {
    alpha = roughnessToAlpha(roughness);
    R0 = std::powf((ior - 1) / (ior + 1), 2);
  }

  inline bool effectivelySmooth() const { return roughness < 1e-3; }

  virtual inline float roughnessToAlpha(float roughness) const {
    return std::powf(roughness, 2);
  }
};

/**
 * Thin layer of rough dielectric microfacet BRDF.
 *
 * Intended to be used in compsition; the transmissive distribution is not
 * defined.
 */
class DielectricCoatBRDF : public Microfacet {
 public:
  DielectricCoatBRDF(float roughness, float ior)
      : Microfacet(Vec3f(1), roughness, ior) {
    distrib = std::make_shared<GGXDistribution>(alpha);
  }

  inline float E(const Vec3f& wo) const override {
    return FresnelSchlick(std::abs(wo.z), R0);
  }

  inline Vec3f f(const Ray& ro, const Ray& ri) override {
    if (IsSpecular(ri.flag)) return Vec3f(1);

    Vec3f wh = ro + ri;
    if (wh.x == 0 && wh.y == 0 && wh.z == 0) return 0;
    wh = wh.normalize();

    float F = FresnelSchlick(dot(ro, wh), R0);
    float G1_schlick = ro.z / (ro.z * (1 - alpha / 2) + alpha / 2);
    float D = distrib->ndf(wh);
    return F * D * G1_schlick / (4 * ri.z * ro.z);
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

    // Transmission
    if (isTransmission) return DistributionSample(Ray::Transmission);

    // Specular Reflection (perfectly smooth surface)
    if (effectivelySmooth()) {
      reflect(ro, ri);
      return DistributionSample(std::make_shared<DiracDeltaDistribution>(ri),
                                Ray::SpecularReflection);
    }

    // Reflection (rough surface using microfacet)
    return DistributionSample(distrib, Ray::Reflection);
  }

  std::optional<BSDFSample> sample(const Ray& ro, const Ray& ri) override {
    if (IsSpecular(ri.flag)) return BSDFSample(Vec3f(1), 0);
    if (ro.z < 1e-6) return {};  // Undefined for transmission
    return BSDFSample(f(ro, ri), distrib->pdf(ro, ri));
  }
};

/**
 * Implementation of "layer" operator in OpenPBR Surface model.
 *
 * This is a composite material produced by bonding 2 materials vertically. The
 * incident ray interacts with the top layer first, and then it interacts with
 * the bottom layer if the scattered ray from the top layer is transmissive.
 *
 * see: https://academysoftwarefoundation.github.io/OpenPBR/#formalism/layering
 */
template <typename Coat, typename Substrate>
class LayerBSDF : public Material {
 public:
  std::unique_ptr<Material> coat;
  std::unique_ptr<Material> substrate;

  LayerBSDF(std::unique_ptr<Coat> coat, std::unique_ptr<Substrate> substrate)
      : Material(MaterialType::BSDF),
        coat(std::move(coat)),
        substrate(std::move(substrate)) {}

  inline Vec3f f(const Ray& ro, const Ray& ri) override {
    float ECoat = coat->E(ro);
    return coat->f(ro, ri) + (1 - ECoat) * substrate->f(ro, ri);
  }

  DistributionSample sampleDistribution(const Ray& ro) override {
    DistributionSample ds = coat->sampleDistribution(ro);
    return IsReflection(ds.flag) ? ds : substrate->sampleDistribution(ro);
  }

  std::optional<BSDFSample> sample(const Ray& ro, const Ray& ri) override {
    if (IsSpecular(ri.flag)) return coat->sample(ro, ri);

    float ECoat = coat->E(ro);
    float ESubstrate = 1 - ECoat;

    Vec3f fValue = coat->f(ro, ri) + substrate->f(ro, ri) * ESubstrate;
    float pdf = coat->distrib->pdf(ro, ri) * ECoat +
                substrate->distrib->pdf(ro, ri) * ESubstrate;
    return BSDFSample(fValue, pdf);
  }
};

/**
 * A variant of "Glossy-diffuse" slab in OpenPBR Surface model[1] where
 * Lambertian model is used for the substrate slab.
 *
 * This is to replicate the look of Principled BSDF Node[2] in Cycles
 * (Blender) with paramerers `Base Color`(albedo), `Roughness`, `IOR`.
 *
 * NOTE: Until Bleder 4.3, Principled BSDF Node used Lambertian as the diffuse
 * substrate. Starting 4.3, Oren-Nayer can be used instead if the new parameter
 * `Diffuse Roughness` is set above 0, and 0 (default value) for Lambertian.[3]
 *
 * [1]https://academysoftwarefoundation.github.io/OpenPBR/#model/basesubstrate/glossy-diffuse
 * [2]https://docs.blender.org/manual/en/4.3/render/shader_nodes/shader/principled.html
 * [3]https://docs.blender.org/manual/en/4.3/render/shader_nodes/shader/principled.html#diffuse
 */
class GlossyDiffuseLambertBSDF
    : public LayerBSDF<DielectricCoatBRDF, LambertBSDF> {
 public:
  GlossyDiffuseLambertBSDF(Vec3f albedo, float roughness, float ior)
      : LayerBSDF(std::make_unique<DielectricCoatBRDF>(roughness, ior),
                  std::make_unique<LambertBSDF>(albedo)) {}
};
