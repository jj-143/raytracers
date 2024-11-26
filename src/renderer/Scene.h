#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "../core/Mesh.h"
#include "../rng.h"
#include "SceneObject.h"

struct Camera {
  Vec3f pos;
  float fov = 50 * M_PI / 180.;
};

struct Intersection {
  Vec3f hit;
  Vec3f n;
  std::shared_ptr<SceneObject> object;
};

class Scene {
 public:
  std::vector<std::shared_ptr<SceneObject>> objects;
  std::vector<std::shared_ptr<LightObject>> lights;
  Camera camera;

  void add(std::shared_ptr<SceneObject> object) {
    this->objects.push_back(object);
  }

  void add(std::shared_ptr<LightObject> light) {
    this->objects.push_back(light);
    this->lights.push_back(light);
  }

  inline std::optional<Intersection> intersect(const Vec3f &origin,
                                               const Vec3f &dir) const {
    float t0 = MAXFLOAT;
    Vec3f hit;
    Vec3f N;
    std::shared_ptr<SceneObject> object;

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
    if (t0 < MAXFLOAT) {
      return Intersection{.hit = hit, .n = N, .object = object};
    }
    return {};
  }

  std::shared_ptr<LightObject> sampleLight() const {
    if (!lights.size()) return nullptr;
    int idx = randInt() % lights.size();
    return lights[idx];
  }
};
