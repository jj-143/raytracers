#pragma once

#include <array>

#include "../math.h"

class Material {
 public:
  Vec3f albedo;

  Material(Vec3f albedo) : albedo(albedo) {}
};

class PhongMaterial : public Material {
 public:
  std::array<float, 3> constants;  // k_specular, k_reflection, k_transmission
  float specularExponent;
  float refractiveIndex;

  PhongMaterial(Vec3f albedo = Vec3f(1, 0, 0),
                std::array<float, 3> constants = {0, 0, 0},
                float specularExponent = 1, float refractiveIndex = 1)
      : Material(albedo),
        constants(constants),
        specularExponent(specularExponent),
        refractiveIndex(refractiveIndex) {}
};
