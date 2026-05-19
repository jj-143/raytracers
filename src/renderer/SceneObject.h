#pragma once

#include <variant>

#include "math.h"

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

  Vec3f L() const { return color * power; }
};

class SceneObject {
  using ObjectType = std::variant<BaseObject, BaseLight>;
  ObjectType obj;

 public:
  int meshId;
  int materialId;

  SceneObject() = default;

  SceneObject(ObjectType obj, int meshId, int materialId)
      : obj(obj), meshId(meshId), materialId(materialId) {}

  template <typename Visitor>
  decltype(auto) visit(Visitor&& visitor) {
    return std::visit(std::forward<Visitor>(visitor), obj);
  }

  template <typename Visitor>
  decltype(auto) visit(Visitor&& visitor) const {
    return std::visit(std::forward<Visitor>(visitor), obj);
  }
};
