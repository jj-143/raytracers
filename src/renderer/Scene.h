#pragma once

#include <cstddef>
#include <limits>
#include <optional>

#include "Allocator.h"
#include "Light.h"
#include "Mesh.h"
#include "math.h"

class SceneObject;

struct Camera {
  Vec3f pos;
  float fov = 50 * M_PI / 180.;
};

struct Intersection {
  Vec3f hit;
  Vec3f n;
  const SceneObject* object;
  std::optional<Light> light;
};

class Scene {
 public:
  Allocator allocator;
  Camera camera;

  inline std::optional<Intersection> intersect(const Vec3f& origin,
                                               const Vec3f& dir) const {
    // TODO: Pass min/max distance from argument.
    float MAX_DIST = std::numeric_limits<float>().max();
    float t0 = MAX_DIST;
    Vec3f hit;
    Vec3f N;
    const SceneObject* object;

    for (size_t i = 0; i < allocator.objectsSize; i++) {
      const SceneObject* obj = &allocator.objects[i];
      const Mesh* mesh = allocator.getMesh(obj);
      float tempDist = 0;
      Vec3f tempN;
      if (mesh->intersect(origin, dir, tempDist, tempN) && tempDist < t0) {
        t0 = tempDist;
        N = tempN;
        hit = dir * t0 + origin;
        object = obj;
      }
    }
    if (t0 < MAX_DIST) {
      return Intersection{
          .hit = hit,
          .n = N,
          .object = object,
          .light =
              IsLight(object) ? std::optional<Light>(object) : std::nullopt,
      };
    }
    return {};
  }

  const SceneObject* sampleLight() const {
    if (allocator.lightsSize == 0) return nullptr;
    int idx = std::min<int>(Sampler::Get1D() * allocator.lightsSize,
                            allocator.lightsSize - 1);
    return &allocator.objects[allocator.lights[idx]];
  }
};
