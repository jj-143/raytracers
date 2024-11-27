#pragma once

#include <memory>

#include "../core/Material.h"
#include "../core/Mesh.h"

class SceneObject {
 public:
  std::shared_ptr<Mesh> target;
  std::shared_ptr<Material> material;

  SceneObject() = default;
  SceneObject(std::shared_ptr<Mesh> m, std::shared_ptr<Material> mat)
      : target(m), material(mat) {}
};

class LightObject : public SceneObject {
 public:
  LightObject(std::shared_ptr<SamplableMesh> m, std::shared_ptr<Emission> mat)
      : SceneObject(m, mat) {}
};

class PointLight : public LightObject {
 public:
  PointLight(Vec3f pos, Vec3f color = Vec3f(1), float strength = 1)
      : LightObject(std::make_shared<SamplableMesh>(pos),
                    std::make_shared<Emission>(color, strength)) {}
};
