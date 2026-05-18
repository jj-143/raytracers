#pragma once

#include <variant>

#include "Material.h"
#include "Mesh.h"

class BaseObject {};

/**
 * BaseLight is just default light implementations acts as point light in
 * WhittedRaytracer, and area light in others.
 */
class BaseLight {
 public:
  Vec3f color;
  float power;

  BaseLight(Vec3f color, float power) : color(color), power(power) {}
};

class SceneObject {
  using ObjectType = std::variant<BaseObject, BaseLight>;
  ObjectType obj;

 public:
  Mesh* mesh;
  Material* material;

  SceneObject() = default;
  SceneObject(ObjectType obj, Mesh* mesh, Material* material)
      : obj(obj), mesh(mesh), material(material) {}

  template <typename Visitor>
  decltype(auto) visit(Visitor&& visitor) {
    return std::visit(std::forward<Visitor>(visitor), obj);
  }
};
