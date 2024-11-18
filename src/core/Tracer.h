#pragma once

#include "../common.h"
#include "../renderer/Scene.h"

class Tracer {
 public:
  virtual inline Vec3f trace(const Scene &scene, Vec3f orig, Vec3f dir) {
    return Vec3f(0);
  }
};

// Whitted style raytracer using Phong illumination model
class WhittedRaytracer : public Tracer {
 public:
  const Vec3f BACKGROUND_COLOR = Vec3f(0, 0, 0);
  const int MAX_REFLECTION_DEPTH = 4;
  const int MAX_REFRACTION_DEPTH = 12;
  const float REFRACTIVE_INDEX_ENVIRONMENT = 1;  // air: 1, water: 1.33

  inline Vec3f trace(const Scene &scene, Vec3f orig, Vec3f dir) override {
    Vec3f color;
    if (!traceBack(scene, orig, dir, color)) {
      return BACKGROUND_COLOR;
    }
    return color;
  }

  inline bool traceBack(const Scene &scene, const Vec3f &orig, const Vec3f dir,
                        Vec3f &color, int depthReflection = 0,
                        int depthRefraction = 0) {
    Vec3f hit = Vec3f(0, 0, 0);
    Vec3f n = Vec3f(0, 0, 0);
    std::shared_ptr<SceneObject> object;

    // Also discard the ray if it exceeds maximum depth as if it doesn't hit
    // anything.
    // Alternative option is to not casting the [reflection | refraction] ray
    // while returning back only the diffuse and specular.
    if (depthReflection > MAX_REFLECTION_DEPTH ||
        depthRefraction > MAX_REFRACTION_DEPTH ||
        !scene.intersect(orig, dir, hit, n, object)) {
      return false;
    };

    float diffIntensity = 0;
    float specIntensity = 0;
    Vec3f r = (dir + n * (dot(dir, n) * -2));

    // reflection
    Vec3f reflectionColor = Vec3f(0, 0, 0);

    Vec3f reflectionDir = r.normalize();
    Vec3f reflectionOrigin = hit + r * 1e-3;

    if (!traceBack(scene, reflectionOrigin, reflectionDir, reflectionColor,
                   depthReflection + 1, depthRefraction)) {
      reflectionColor = BACKGROUND_COLOR;
    };

    // Refraction
    Vec3f refractionColor = Vec3f(0, 0, 0);
    if (object->material->albedo[3] > 0) {
      bool isInside = dot(dir, n) > 0;
      float indexI = isInside ? object->material->refractiveIndex
                              : REFRACTIVE_INDEX_ENVIRONMENT;
      float indexT = isInside ? REFRACTIVE_INDEX_ENVIRONMENT
                              : object->material->refractiveIndex;

      Vec3f dirRefraction;
      if (refract(dir, isInside ? -n : n, indexI, indexT, dirRefraction)) {
        dirRefraction = dirRefraction.normalize();
        if (!traceBack(scene, hit + dirRefraction * 1e-3, dirRefraction,
                       refractionColor, depthReflection, depthRefraction + 1)) {
          refractionColor = BACKGROUND_COLOR;
        }
      }
    }

    for (auto &light : scene.lights) {
      Vec3f dirLight = (light->pos - hit).normalize();

      // shadow
      Vec3f tempHit = Vec3f(0, 0, 0);
      Vec3f tempN = Vec3f(0, 0, 0);
      std::shared_ptr<SceneObject> tempObject;
      Vec3f shadowOrigin =
          dot(dirLight, n) < 0 ? hit - n * 1e-3 : hit + n * 1e-3;
      if (scene.intersect(shadowOrigin, dirLight, tempHit, tempN, tempObject)) {
        float dist = (tempHit - shadowOrigin).norm();
        if (dist < (light->pos - hit).norm()) {
          continue;
        }
      }

      diffIntensity += std::max(0.f, dot(n, dirLight)) * light->intensity;

      specIntensity += powf(std::max(0.f, dot(r, dirLight)),
                            object->material->specularExponent) *
                       light->intensity;
    }

    color = object->material->diffuseColor * diffIntensity *
                object->material->albedo[0] +
            Vec3f(1, 1, 1) * specIntensity * object->material->albedo[1] +
            reflectionColor * object->material->albedo[2] +
            refractionColor * object->material->albedo[3];
    return true;
  }
};
