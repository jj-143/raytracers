#pragma once

#include <utility>

#include "SceneObject.h"

class Mesh;
class Material;
class LightObject;

class Allocator {
 public:
  Mesh* meshes[128];
  Material* materials[128];
  SceneObject* objects[128];
  SceneObject* lights[128];
  int meshesSize = 0;
  int materialsSize = 0;
  int objectsSize = 0;
  int lightsSize = 0;

  template <typename T, typename... Args>
  T* emplaceMesh(Args&&... args) {
    T* ptr = new T(std::forward<Args>(args)...);
    meshes[meshesSize++] = ptr;
    return ptr;
  }

  template <typename T, typename... Args>
  T* emplaceMaterial(Args&&... args) {
    T* ptr = new T(std::forward<Args>(args)...);
    materials[materialsSize++] = ptr;
    return ptr;
  }

  template <typename T>
  SceneObject* emplaceObject(Mesh* mesh, Material* material) {
    SceneObject* ptr = new SceneObject(T(), mesh, material);
    objects[objectsSize++] = ptr;

    if constexpr (std::is_same_v<T, Light>) {
      lights[lightsSize++] = ptr;
    }

    return ptr;
  }
};
