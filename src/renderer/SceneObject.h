#pragma once

#include <variant>

#include "Material.h"
#include "Mesh.h"

class BaseObject {};

/**
 * NOTE: Currently Light in WhittedRaytracer is considered as point light,
 * and area light for others. No Light type implementations yet.
 */
class Light {};

class SceneObject {
  using ObjectType = std::variant<BaseObject, Light>;
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
