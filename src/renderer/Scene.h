#pragma once

#include <memory>
#include <vector>

#include "../common.h"
#include "../core/Mesh.h"

class Scene {
 public:
  std::vector<std::shared_ptr<SceneObject>> objects;
  std::vector<std::shared_ptr<PointLight>> lights;

  void add(std::shared_ptr<SceneObject> object) {
    this->objects.push_back(object);
  }

  void add(std::shared_ptr<PointLight> light) { this->lights.push_back(light); }

  inline bool intersect(const Vec3f &origin, const Vec3f &dir, Vec3f &hit,
                        Vec3f &N, std::shared_ptr<SceneObject> &object) const {
    float t0 = MAXFLOAT;

    for (auto &obj : objects) {
      const Mesh &mesh = *obj->target;
      float tempDist = 0;
      Vec3f tempN;
      if (mesh.intersect(origin, dir, tempDist, tempN) && tempDist < t0) {
        t0 = tempDist;
        N = tempN;
        hit = dir * t0 + origin;
        object = obj;
      }
    }
    return t0 < MAXFLOAT;
  }
};
