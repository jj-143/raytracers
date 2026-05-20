#pragma once

#include "compat.h"
#include "math.h"
#include "raytracers.h"

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

  RT_DEVICE_HOST Vec3f L() const { return color * power; }
};

class SceneObject {
  using ObjectType = compat::variant<BaseObject, BaseLight>;
  ObjectType obj;

 public:
  int meshId;
  int materialId;

  SceneObject() = default;

  SceneObject(ObjectType obj, int meshId, int materialId)
      : obj(obj), meshId(meshId), materialId(materialId) {}

  template <typename Visitor>
  RT_DEVICE_HOST decltype(auto) visit(Visitor&& visitor) {
    return compat::visit(std::forward<Visitor>(visitor), obj);
  }

  template <typename Visitor>
  RT_DEVICE_HOST decltype(auto) visit(Visitor&& visitor) const {
    return compat::visit(std::forward<Visitor>(visitor), obj);
  }
};
