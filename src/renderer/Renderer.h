#pragma once

#include <memory>

#include "../common.h"
#include "../core/Tracer.h"
#include "../math.h"
#include "../utils.h"
#include "Scene.h"

struct RenderConfig {
  int width;
  int height;
  int spp;
};

class Renderer {
 public:
  RenderConfig config;
  std::shared_ptr<Tracer> tracer;
  std::vector<Vec3f> framebuffer;

  Renderer(std::shared_ptr<Tracer> tracer) : tracer(tracer) {};

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
          float x = (2 * (i + 0.5) / (float)config.width - 1) * tan(fov / 2.);
          float y = -(2 * (j + 0.5) / (float)config.height - 1) *
                    tan(fov / 2.) * config.height / (float)config.width;

          // Random sample inside the pixel;
          x += (randf() - .5) * pixelSize;
          y += (randf() - .5) * pixelSize;

          Vec3f dir = Vec3f(x, y, -1).normalize();

          Vec3f sampleColor = tracer->trace(scene, camOrig, dir);
          color = color + sampleColor;
        }
        color = color * (1.f / config.spp);
        framebuffer[i + j * config.width] = color;
      }
    }

    std::cout << "Render took " << (nowMillis() - now) << "ms" << std::endl;
  }
};
