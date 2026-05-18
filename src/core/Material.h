#pragma once

#include <array>
#include <optional>
#include <variant>

#include "Distribution.h"
#include "Sampler.h"
#include "common.h"
#include "math.h"

class PhongMaterial {
 public:
  Vec3f albedo;
  std::array<float, 3> constants;  // k_specular, k_reflection, k_transmission
  float specularExponent;
  float refractiveIndex;

  PhongMaterial(Vec3f albedo = Vec3f(1, 0, 0),
                std::array<float, 3> constants = {0, 0, 0},
                float specularExponent = 1, float refractiveIndex = 1)
      : albedo(albedo),
        constants(constants),
        specularExponent(specularExponent),
        refractiveIndex(refractiveIndex) {}

  // Not used
  Vec3f f(auto, auto) const { return {}; }
  Ray sampleRi(auto) const { return {}; }
  std::optional<BSDFSample> sample(auto, auto) const { return {}; }
};

class LambertBSDF {
 public:
  Vec3f albedo;
  CosineDistribution dist;

  LambertBSDF(Vec3f albedo) : albedo(albedo) {}

  Vec3f f(const Ray& ro, const Ray& ri) const { return albedo / M_PI; }

  Ray sampleRi(const Ray& ro) const { return {dist.sample(ro), Ray::Diffuse}; }

  std::optional<BSDFSample> sample(const Ray& ro, const Ray& ri) const {
    return BSDFSample(f(ro, ri), dist.pdf(ro, ri));
  }
};

class MetalBSDF {
 public:
  Vec3f albedo;

  MetalBSDF(Vec3f albedo) : albedo(albedo) {}

  Vec3f f(const Ray& ro, const Ray& ri) const {
    return IsSpecular(ri.flag) ? albedo : 0;
  }

  Ray sampleRi(const Ray& ro) const {
    Vec3f reflected;
    reflect(ro, reflected);
    return {DiracDeltaDistribution(reflected).sample(ro), Ray::Specular};
  }

  std::optional<BSDFSample> sample(const Ray& ro, const Ray& ri) const {
    return BSDFSample(f(ro, ri), 0);
  }
};

class DielectricBSDF {
 public:
  float ior;  // Like in Blender, it's a relative value to surroundings.
  float R0;   // Reflectance at wi = 0

  DielectricBSDF(float ior) : ior(ior) {
    R0 = std::powf((ior - 1) / (ior + 1), 2);
  }

  Vec3f f(const Ray& ro, const Ray& ri) const {
    return IsSpecular(ri.flag) ? 1 : 0;
  }

  Ray sampleRi(const Ray& ro) const {
    Ray ri;
    float VoH = ro.z;
    bool isInside = VoH < 0;
    float eta = isInside ? ior : 1.f / ior;
    float R = FresnelSchlick(std::abs(VoH), R0);
    Vec3f N = isInside ? Vec3f(0, 0, -1) : Vec3f(0, 0, 1);

    bool isAllowedAngle = refract(-ro, N, eta, ri);
    bool isTransmission = isAllowedAngle && Sampler::Get1D() < (1 - R);

    if (isTransmission) {
      ri.normalize();
    } else {
      reflect(ro, ri);
    }

    return {
        DiracDeltaDistribution(ri).sample(ro),
        isTransmission ? Ray::SpecularTransmission : Ray::SpecularReflection};
  }

  std::optional<BSDFSample> sample(const Ray& ro, const Ray& ri) const {
    return BSDFSample(f(ro, ri), 0);
  }
};

class Microfacet {
 public:
  static bool EffectivelySmooth(float roughness) { return roughness < 1e-3; }

  static float RoughnessToAlpha(float roughness) {
    return std::powf(roughness, 2);
  }
};

/**
 * Thin layer of rough dielectric microfacet BRDF.
 *
 * Intended to be used in compsition; the transmissive distribution is not
 * defined.
 */
class DielectricCoatBRDF {
  float roughness;
  float alpha;
  float ior;  // see: `DielectricBSDF`
  float R0;   // Reflectance at wi = 0

 public:
  GGXDistribution dist;

  DielectricCoatBRDF(float roughness, float ior)
      : roughness(roughness),
        ior(ior),
        alpha(Microfacet::RoughnessToAlpha(roughness)),
        R0(std::powf((ior - 1) / (ior + 1), 2)),
        dist(alpha) {}

  float E(const Vec3f& wo) const { return FresnelSchlick(std::abs(wo.z), R0); }

  Vec3f f(const Ray& ro, const Ray& ri) const {
    if (IsSpecular(ri.flag)) return Vec3f(1);

    Vec3f wh = ro + ri;
    if (wh.x == 0 && wh.y == 0 && wh.z == 0) return 0;
    wh = wh.normalize();

    float F = FresnelSchlick(dot(ro, wh), R0);
    float G1_schlick = ro.z / (ro.z * (1 - alpha / 2) + alpha / 2);
    float D = dist.ndf(wh);
    return F * D * G1_schlick / (4 * ri.z * ro.z);
  }

  Ray sampleRi(const Ray& ro) const {
    Ray ri;
    float VoH = ro.z;
    bool isInside = VoH < 0;
    float eta = isInside ? ior : 1.f / ior;
    float R = FresnelSchlick(std::abs(VoH), R0);
    Vec3f N = isInside ? Vec3f(0, 0, -1) : Vec3f(0, 0, 1);

    bool isAllowedAngle = refract(-ro, N, eta, ri);
    bool isTransmission = isAllowedAngle && Sampler::Get1D() < (1 - R);

    // Transmission
    if (isTransmission) return {{}, Ray::Transmission};

    // Specular Reflection (perfectly smooth surface)
    if (Microfacet::EffectivelySmooth(roughness)) {
      reflect(ro, ri);
      return {DiracDeltaDistribution(ri).sample(ro), Ray::SpecularReflection};
    }

    // Reflection (rough surface using microfacet)
    return {dist.sample(ro), Ray::Reflection};
  }

  std::optional<BSDFSample> sample(const Ray& ro, const Ray& ri) const {
    if (IsSpecular(ri.flag)) return BSDFSample(Vec3f(1), 0);
    if (ro.z < 1e-6) return {};  // Undefined for transmission
    return BSDFSample(f(ro, ri), dist.pdf(ro, ri));
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
class LayerBSDF {
 public:
  Coat coat;
  Substrate substrate;

  LayerBSDF(Coat coat, Substrate substrate)
      : coat(coat), substrate(substrate) {}

  Vec3f f(const Ray& ro, const Ray& ri) const {
    float ECoat = coat.E(ro);
    return coat.f(ro, ri) + (1 - ECoat) * substrate.f(ro, ri);
  }

  Ray sampleRi(const Ray& ro) const {
    Ray ri = coat.sampleRi(ro);
    if (IsReflection(ri.flag)) return ri;
    return IsReflection(ri.flag) ? ri : substrate.sampleRi(ro);
  }

  std::optional<BSDFSample> sample(const Ray& ro, const Ray& ri) const {
    if (IsSpecular(ri.flag)) return coat.sample(ro, ri);

    float ECoat = coat.E(ro);
    float ESubstrate = 1 - ECoat;

    Vec3f fValue = coat.f(ro, ri) + substrate.f(ro, ri) * ESubstrate;
    float pdf =
        coat.dist.pdf(ro, ri) * ECoat + substrate.dist.pdf(ro, ri) * ESubstrate;
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
      : LayerBSDF(DielectricCoatBRDF(roughness, ior), LambertBSDF(albedo)) {}
};

class Material {
  using MaterialType = std::variant<PhongMaterial, LambertBSDF, MetalBSDF,
                                    DielectricBSDF, GlossyDiffuseLambertBSDF>;
  MaterialType mat;

 public:
  Material(MaterialType mat) : mat(mat) {}

  Vec3f f(const Ray& ro, const Ray& ri) const {
    return std::visit([ro, ri](auto&& mat) -> Vec3f { return mat.f(ro, ri); },
                      mat);
  }

  Ray sampleRi(const Ray& ro) const {
    return std::visit([ro](auto&& mat) -> Ray { return mat.sampleRi(ro); },
                      mat);
  }

  std::optional<BSDFSample> sample(const Ray& ro, const Ray& ri) const {
    return std::visit(
        [ro, ri](auto&& mat) -> std::optional<BSDFSample> {
          return mat.sample(ro, ri);
        },
        mat);
  }
};
