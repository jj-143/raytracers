#pragma once

#include <memory>
#include <string>

#include "core/Material.h"
#include "core/Mesh.h"
#include "math.h"

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
