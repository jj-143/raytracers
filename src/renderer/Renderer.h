#pragma once

#include <memory>

#include "../core/Sampler.h"
#include "../math.h"
#include "../utils.h"
#include "Scene.h"
#include "Tracer.h"

struct RenderConfig {
  int width;
  int height;
  int spp;
  int seed;
};

class Renderer {
 public:
  RenderConfig config;
  std::shared_ptr<Tracer> tracer;
  std::vector<Vec3f> framebuffer;
  std::unique_ptr<Sampler::Sampler> sampler;

  Renderer(std::shared_ptr<Tracer> tracer) : tracer(tracer) {};

  inline void render(const Scene& scene) {
    float halfFov = scene.camera.fov / 2.;
    float pixelSize = tan(halfFov) / config.width;

    this->framebuffer.resize(config.width * config.height);

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

          Vec3f sampleColor = tracer->trace(scene, scene.camera.pos, dir);
          color = color + sampleColor;
        }
        color = color * (1.f / sampler->spp);
        framebuffer[i + j * config.width] = color;
      }
    }

    std::cout << "Render took " << (nowMillis() - now) << "ms" << std::endl;

    // Gamma Encoding
    for (size_t i = 0; i < config.height * config.width; ++i) {
      for (size_t j = 0; j < 3; j++) {
        if (framebuffer[i][j] != framebuffer[i][j]) {
          framebuffer[i][j] = 0;  // NaN fix
        }
        framebuffer[i][j] = gammaTransform(framebuffer[i][j], 2.2);
      }
    }
  }
};
