#pragma once

#include <utility>

class Mesh;
class SceneObject;
class Material;
class LightObject;

class Allocator {
 public:
  Mesh* meshes[128];
  Material* materials[128];
  SceneObject* objects[128];
  LightObject* lights[128];
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

  template <typename T, typename... Args>
  T* emplaceObject(Args&&... args) {
    T* ptr = new T(std::forward<Args>(args)...);
    objects[objectsSize++] = ptr;

    if constexpr (std::is_same_v<T, LightObject>) {
      lights[lightsSize++] = ptr;
    }

    return ptr;
  }
};
