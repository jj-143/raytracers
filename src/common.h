#pragma once

#include <array>
#include <memory>
#include <string>

#include "core/Mesh.h"
#include "math.h"

struct Material {
  Vec3f albedo;
  std::array<float, 3> constants;  // k_specular, k_reflection, k_transmission
  float specularExponent;
  float refractiveIndex;
};

class SceneObject {
 public:
  std::shared_ptr<Mesh> target;
  std::shared_ptr<Material> material;

  SceneObject() = default;
  SceneObject(std::shared_ptr<Mesh> m, std::shared_ptr<Material> mat)
      : target(m), material(mat) {}
};

struct PointLight {
  Vec3f pos;
  Vec3f lightColor = Vec3f(1);
};
