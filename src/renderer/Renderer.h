#pragma once

#include <memory>

#include "../common.h"
#include "../geometry.h"
#include "../utils.h"
#include "Scene.h"

const Vec3f BACKGROUND_COLOR = Vec3f(0, 0, 0);
const int MAX_REFLECTION_DEPTH = 4;
const int MAX_REFRACTION_DEPTH = 12;

const float REFRACTIVE_INDEX_ENVIRONMENT = 1;  // air: 1, water: 1.33

bool refract(Vec3f d, Vec3f N, float nI, float nT, Vec3f &dir) {
  float n = nI / nT;
  float cosI = -d * N;
  float sinI = sqrtf(1 - cosI * cosI);
  if (sinI > 1 / n) return false;  // total reflection

  float sinT = n * sinI;
  float cosT = sqrtf(1 - sinT * sinT);

  Vec3f dH = (d + N * cosI).normalize();
  Vec3f dN = -N;

  dir = dH * sinT + dN * cosT;
  return true;
}

bool castRay(const Scene &scene, const Vec3f &orig, const Vec3f dir,
             Vec3f &color, int depthReflection = 0, int depthRefraction = 0) {
  Vec3f hit = Vec3f(0, 0, 0);
  Vec3f n = Vec3f(0, 0, 0);
  std::shared_ptr<Object> object;

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
  Vec3f r = (dir + n * ((dir * n) * -2));

  // reflection
  Vec3f reflectionColor = Vec3f(0, 0, 0);

  Vec3f reflectionDir = r.normalize();
  Vec3f reflectionOrigin = hit + r * 1e-3;

  if (!castRay(scene, reflectionOrigin, reflectionDir, reflectionColor,
               depthReflection + 1, depthRefraction)) {
    reflectionColor = BACKGROUND_COLOR;
  };

  // Refraction
  Vec3f refractionColor = Vec3f(0, 0, 0);
  if (object->material->albedo[3] > 0) {
    bool isInside = (dir * n) > 0;
    float indexI = isInside ? object->material->refractiveIndex
                            : REFRACTIVE_INDEX_ENVIRONMENT;
    float indexT = isInside ? REFRACTIVE_INDEX_ENVIRONMENT
                            : object->material->refractiveIndex;

    Vec3f dirRefraction;
    if (refract(dir, isInside ? -n : n, indexI, indexT, dirRefraction)) {
      dirRefraction = dirRefraction.normalize();
      if (!castRay(scene, hit + dirRefraction * 1e-3, dirRefraction,
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
    std::shared_ptr<Object> tempObject;
    Vec3f shadowOrigin = dirLight * n < 0 ? hit - n * 1e-3 : hit + n * 1e-3;
    if (scene.intersect(shadowOrigin, dirLight, tempHit, tempN, tempObject)) {
      float dist = (tempHit - shadowOrigin).norm();
      if (dist < (light->pos - hit).norm()) {
        continue;
      }
    }

    diffIntensity += std::max(0.f, n * dirLight) * light->intensity;

    specIntensity +=
        powf(std::max(0.f, r * dirLight), object->material->specularExponent) *
        light->intensity;
  }

  color = object->material->diffuseColor * diffIntensity *
              object->material->albedo[0] +
          Vec3f(1, 1, 1) * specIntensity * object->material->albedo[1] +
          reflectionColor * object->material->albedo[2] +
          refractionColor * object->material->albedo[3];
  return true;
}

struct RenderConfig {
  int width;
  int height;
  int spp;
};

class Renderer {
 public:
  RenderConfig config;
  std::vector<Vec3f> framebuffer;

  inline void render(const Scene &scene) {
    this->framebuffer.resize(config.width * config.height);

    // Render Pass
    int64_t now = nowMillis();

    // Camera stuff
    Vec3f camOrig = {.275, .275, .8};

    // 35mm lens with 25mm sensor
    float fov = 39.3076 * M_PI / 180;
    float pixelSize = tan(fov / 2) / config.width;

#pragma omp parallel for collapse(2)
    for (size_t j = 0; j < config.height; j++) {
      for (size_t i = 0; i < config.width; i++) {
        Vec3f color = Vec3f();

#pragma omp parallel for
        for (size_t iSample = 0; iSample < config.spp; iSample++) {
          Vec3f sampleColor = Vec3f();
          float x = (2 * (i + 0.5) / (float)config.width - 1) * tan(fov / 2.);
          float y = -(2 * (j + 0.5) / (float)config.height - 1) *
                    tan(fov / 2.) * config.height / (float)config.width;

          // Random sample inside the pixel;
          x += (randf() - .5) * pixelSize;
          y += (randf() - .5) * pixelSize;

          Vec3f dir = Vec3f(x, y, -1).normalize();

          if (!castRay(scene, camOrig, dir, sampleColor)) {
            sampleColor = BACKGROUND_COLOR;
          }
          color = color + sampleColor;
        }
        color = color * (1.f / config.spp);
        framebuffer[i + j * config.width] = color;
      }
    }

    std::cout << "Render took " << (nowMillis() - now) << "ms" << std::endl;
  }
};
