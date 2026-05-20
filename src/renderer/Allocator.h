#pragma once

#include <utility>

#include "Material.h"
#include "Mesh.h"
#include "SceneObject.h"
#include "raytracers.h"

class LightObject;

class Allocator {
 public:
  Mesh meshes[128];
  Material materials[128];
  SceneObject objects[128];
  int lights[128];

  int meshIndex[128];
  int materialIndex[128];

  int meshesSize = 0;
  int materialsSize = 0;
  int objectsSize = 0;
  int lightsSize = 0;

  RT_DEVICE_HOST const Material* getMaterial(const SceneObject* obj) const {
    return &materials[obj->materialId];
  }

  RT_DEVICE_HOST const Mesh* getMesh(const SceneObject* obj) const {
    return &meshes[obj->meshId];
  }

  template <typename T, typename... Args>
  int emplaceMesh(Args&&... args) {
    std::construct_at(&meshes[meshesSize], T(std::forward<Args>(args)...));
    return meshesSize++;
  }

  template <typename T, typename... Args>
  int emplaceMaterial(Args&&... args) {
    std::construct_at(&materials[materialsSize],
                      T(std::forward<Args>(args)...));
    return materialsSize++;
  }

  template <typename T, typename... Args>
  SceneObject* emplaceObject(int meshId, int materialId, Args&&... args) {
    meshIndex[objectsSize] = meshId;
    materialIndex[objectsSize] = materialId;

    std::construct_at(&objects[objectsSize], T(std::forward<Args>(args)...),
                      meshId, materialId);

    if constexpr (compat::is_same_v<T, BaseLight>) {
      lights[lightsSize++] = objectsSize;
    }

    return &objects[objectsSize++];
  }
};
