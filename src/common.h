#pragma once

#include <memory>
#include <string>

#include "core/Mesh.h"
#include "math.h"

struct Material {
  Vec3f diffuseColor;
  std::vector<float> albedo = {1, 0, 0, 0};
  float specularExponent;
  float refractiveIndex = 1;
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
  float intensity;
};
