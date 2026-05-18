#pragma once

#include <utility>

#include "Light.h"
#include "Material.h"
#include "Mesh.h"
#include "SceneObject.h"

class LightObject;

class Allocator {
 public:
  Mesh meshes[128];
  Material materials[128];
  SceneObject objects[128];
  SceneObject* lights[128];
  int meshesSize = 0;
  int materialsSize = 0;
  int objectsSize = 0;
  int lightsSize = 0;

  template <typename T, typename... Args>
  Mesh* emplaceMesh(Args&&... args) {
    meshes[meshesSize] = {T(std::forward<Args>(args)...)};
    return &meshes[meshesSize++];
  }

  template <typename T, typename... Args>
  Material* emplaceMaterial(Args&&... args) {
    materials[materialsSize] = {T(std::forward<Args>(args)...)};
    return &materials[materialsSize++];
  }

  template <typename T, typename... Args>
  SceneObject* emplaceObject(Mesh* mesh, Material* material, Args&&... args) {
    objects[objectsSize] = {T(std::forward<Args>(args)...), mesh, material};
    SceneObject* ptr = &objects[objectsSize++];

    if constexpr (std::is_same_v<T, BaseLight>) {
      lights[lightsSize++] = ptr;
    }

    return ptr;
  }
};
