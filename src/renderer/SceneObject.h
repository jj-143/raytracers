#pragma once

#include <memory>

#include "Material.h"
#include "Mesh.h"

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
  std::shared_ptr<Mesh> mesh;

  LightObject(std::shared_ptr<Mesh> m, std::shared_ptr<Emission> mat)
      : SceneObject(m, mat), mesh(m) {}
};

class PointLight : public LightObject {
 public:
  PointLight(Vec3f pos, Vec3f color = Vec3f(1), float strength = 1)
      : LightObject(std::make_shared<Mesh>(pos),
                    std::make_shared<Emission>(color, strength)) {}
};
