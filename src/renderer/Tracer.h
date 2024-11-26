#pragma once

#include <optional>

#include "../core/Material.h"
#include "../onb.h"
#include "../rng.h"
#include "Scene.h"

class Tracer {
 public:
  virtual inline Vec3f trace(const Scene &scene, Vec3f orig, Vec3f dir) {
    return Vec3f(0);
  }
};

/**
 * Whitted style recursive raytracer using Phong illumination model, based on
 * `tinyraytracer`. Should be used with [PhongMaterial]
 */
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
    // Also discard the ray if it exceeds maximum depth as if it doesn't hit
    // anything.
    // Alternative option is to not casting the reflection or refraction ray
    // while returning back only the diffuse and specular.
    if (depthReflection > MAX_REFLECTION_DEPTH ||
        depthRefraction > MAX_REFRACTION_DEPTH) {
      return false;
    };

    std::optional<Intersection> intr = scene.intersect(orig, dir);
    if (!intr) return false;

    Vec3f &hit = intr->hit;
    Vec3f &n = intr->n;
    PhongMaterial &material =
        *std::static_pointer_cast<PhongMaterial>(intr->object->material);

    // Casting reflection ray
    Vec3f reflectionColor = Vec3f(0, 0, 0);
    Vec3f reflectionDir = (dir + n * (dot(dir, n) * -2)).normalize();
    Vec3f reflectionOrigin = hit + reflectionDir * 1e-3;

    if (!traceBack(scene, reflectionOrigin, reflectionDir, reflectionColor,
                   depthReflection + 1, depthRefraction)) {
      reflectionColor = BACKGROUND_COLOR;
    };

    // Casting transmission ray (refraction)
    Vec3f refractionColor = Vec3f(0, 0, 0);
    if (material.constants[2] > 0) {
      bool isInside = dot(dir, n) > 0;
      float eta = isInside
                      ? material.refractiveIndex / REFRACTIVE_INDEX_ENVIRONMENT
                      : REFRACTIVE_INDEX_ENVIRONMENT / material.refractiveIndex;
      Vec3f dirRefraction;

      if (refract(dir, isInside ? -n : n, eta, dirRefraction)) {
        dirRefraction = dirRefraction.normalize();
        if (!traceBack(scene, hit + dirRefraction * 1e-3, dirRefraction,
                       refractionColor, depthReflection, depthRefraction + 1)) {
          refractionColor = BACKGROUND_COLOR;
        }
      }
    }

    // Calculate diffuse & specular contributions from each light.
    Vec3f diffColor;
    Vec3f specColor;

    for (auto &light : scene.lights) {
      Vec3f dirLight = (light->target->pos - hit).normalize();
      float NoL = dot(n, dirLight);

      // Casting shadow ray
      Vec3f shadowOrigin = NoL < 0 ? hit - n * 1e-3 : hit + n * 1e-3;
      if (auto intr = scene.intersect(shadowOrigin, dirLight)) {
        float dHit = (intr->hit - shadowOrigin).norm();
        float dLight = (light->target->pos - shadowOrigin).norm();
        if (dLight - dHit > 1e-3) {
          continue;
        }
      }

      diffColor = diffColor + std::max(0.f, NoL) * light->material->albedo;

      specColor = specColor + powf(std::max(0.f, dot(reflectionDir, dirLight)),
                                   material.specularExponent) *
                                  light->material->albedo;
    }

    color = material.albedo * diffColor + material.constants[0] * specColor +
            material.constants[1] * reflectionColor +
            material.constants[2] * refractionColor;
    return true;
  }
};

/**
 * A recursive pathtracer using Monte Carlo and mixed sampling method, based on
 * `Ray Tracing in One Weekend` series `Ray Tracing: The Rest of Your Life`.
 *
 * Light ray `ri` is randomly chosen between BSDF sampling and light sampling
 * with equal probability.
 * For non-specular rays, PDF value is equally weighted average of their pdfs.
 * For specular rays, only BSDF sampling is performed for efficiency, since
 * pdf is always 0 for Dirac Delta distribution, effectively wasting 50% of the
 * samples.
 */
class RecursivePathtracer : public Tracer {
 public:
  int maxDepth = 8;

  Vec3f trace(const Scene &scene, Vec3f orig, Vec3f dir) override {
    return traceBack(orig, dir, scene);
  }

  Vec3f traceBack(const Vec3f &orig, const Vec3f dir, const Scene &scene,
                  int depth = 0) {
    if (depth > maxDepth) return Vec3f(0);

    std::optional<Intersection> intr = scene.intersect(orig, dir);
    if (!intr) return Vec3f(0);

    Vec3f &hit = intr->hit;
    Material &bsdf = *intr->object->material;
    ONB onb = ONB(intr->n);           // orthonormal basis with surface normal
    Ray ro = onb.toLocalSpace(-dir);  // view ray "outgoing ray"
    Ray ri;                           // light ray "incoming ray"
    Vec3f wiW;                        // light ray, world space

    if (bsdf.type == MaterialType::Emission) return ro.z > 0 ? bsdf.albedo : 0;

    DistributionSample ds = bsdf.sampleDistribution(ro);

    if (IsSpecular(ds.flag)) {
      ri = ds.sample(ro);
      std::optional<BSDFSample> bs = bsdf.sample(ro, ri);
      if (!bs) return Vec3f(0);
      Vec3f wiW = onb.toWorldSpace(ri);
      Vec3f Li = traceBack(hit + wiW * 1e-3, wiW, scene, depth + 1);
      return bs->fValue * Li;
    }

    bool isLightSampling = randf() < .5;
    const auto &light = scene.sampleLight();

    if (isLightSampling && light) {
      wiW = light->sampler->generate(hit);
      ri = onb.toLocalSpace(wiW);
    } else {
      ri = ds.sample(ro);
      wiW = onb.toWorldSpace(ri);
    }

    std::optional<BSDFSample> bs = bsdf.sample(ro, ri);
    if (!bs) return Vec3f(0);
    float NoL = ri.z;
    if (NoL < 0) return Vec3f(0);

    float lightPdf = light ? light->sampler->pdf(wiW, hit) : 0;
    float pdf = (lightPdf + bs->pdf) / 2;
    if (pdf < 1e-6) return Vec3f(0);

    Vec3f Li = traceBack(hit + wiW * 1e-3, wiW, scene, depth + 1);
    return bs->fValue * NoL * Li / pdf;
  }
};
