#pragma once

#include <concepts>

#include "SceneObject.h"
#include "helpers.h"

template <typename T>
concept Emitter = requires(T t) {
  { t.L() } -> std::same_as<Vec3f>;
};

bool constexpr IsLight(const SceneObject* obj) {
  if (!obj) return false;
  return obj->visit(overloaded{
      [](Emitter auto&&) { return true; },
      [](auto&&) { return false; },
  });
}

// Light interface to use with SceneObject
class Light {
 public:
  Light(const SceneObject* obj) : obj(obj) {}

  Vec3f L() {
    return obj->visit(overloaded{
        [](Emitter auto&& l) { return l.L(); },
        [](auto&&) { return Vec3f{0}; },
    });
  }

 private:
  const SceneObject* obj;
};
