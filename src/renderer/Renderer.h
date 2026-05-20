#pragma once

#include <memory>

#include "Sampler.h"
#include "Scene.h"
#include "Tracer.h"
#include "math.h"
#include "utils.h"

struct RenderConfig {
  int width;
  int height;
  int spp;
  int seed;
  Tracer::Type tracerType;
};

class Renderer {
 public:
  RenderConfig config;
  Tracer tracer;
  float* framebuffer = nullptr;

  Renderer(RenderConfig config)
      : config(config), tracer(Tracer::Create(config.tracerType)) {}

  inline void render(const Scene& scene) {
    float halfFov = scene.camera.fov / 2.;
    float pixelSize = tan(halfFov) / config.width;

    initFramebuffer(config.width * config.height);

    int64_t now = nowMillis();

#pragma omp parallel for schedule(static)
    for (size_t j = 0; j < config.height; j++) {
      for (size_t i = 0; i < config.width; i++) {
        auto sampler = Sampler::InitSampler(config.spp, config.seed);
        Vec3f color = Vec3f();

        for (size_t iSample = 0; iSample < sampler->spp; iSample++) {
          sampler->startPixelSample({(int)i, (int)j}, iSample);

          float x = (2 * (i + 0.5) / (float)config.width - 1) * tan(halfFov);
          float y = -(2 * (j + 0.5) / (float)config.height - 1) * tan(halfFov) *
                    config.height / (float)config.width;

          Vec2f uv = sampler->get2D();
          x += (uv.x - .5) * pixelSize;
          y += (uv.y - .5) * pixelSize;

          Vec3f dir = Vec3f(x, y, -1).normalize();

          Vec3f sampleColor = tracer.trace(&scene, scene.camera.pos, dir);
          color = color + sampleColor;
        }
        color = color * (1.f / sampler->spp);

        int idx = 3 * (j * config.width + i);

        framebuffer[idx] = color[0];
        framebuffer[idx + 1] = color[1];
        framebuffer[idx + 2] = color[2];
      }
    }

    std::cout << "Render took " << (nowMillis() - now) << "ms" << std::endl;

    gammaEncode(framebuffer, config.width, config.height, 2.2);
  }

  void destroy() {
    if (framebuffer) free(framebuffer);
  }

 private:
  void initFramebuffer(int size) {
    framebuffer = (float*)malloc(3 * size * sizeof(float));
  }
};
