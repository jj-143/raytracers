#pragma once

#include "Allocator.h"
#include "Material.h"
#include "Sampler.h"
#include "Scene.h"
#include "SceneObject.h"
#include "compat.h"
#include "onb.h"

/**
 * Whitted style recursive raytracer using Phong illumination model, based on
 * `tinyraytracer`. Should be used with [PhongMaterial]
 */
class WhittedRaytracer {
 public:
  const Vec3f BACKGROUND_COLOR = Vec3f(0, 0, 0);
  const int MAX_REFLECTION_DEPTH = 4;
  const int MAX_REFRACTION_DEPTH = 12;
  const float REFRACTIVE_INDEX_ENVIRONMENT = 1;  // air: 1, water: 1.33

  inline Vec3f trace(const Scene& scene, Vec3f orig, Vec3f dir) {
    Vec3f color;
    if (!traceBack(scene, orig, dir, color)) {
      return BACKGROUND_COLOR;
    }
    return color;
  }

  inline bool traceBack(const Scene& scene, const Vec3f& orig, const Vec3f dir,
                        Vec3f& color, int depthReflection = 0,
                        int depthRefraction = 0) {
    // Also discard the ray if it exceeds maximum depth as if it doesn't hit
    // anything.
    // Alternative option is to not casting the reflection or refraction ray
    // while returning back only the diffuse and specular.
    if (depthReflection > MAX_REFLECTION_DEPTH ||
        depthRefraction > MAX_REFRACTION_DEPTH) {
      return false;
    };

    const Allocator& alloc = scene.allocator;

    compat::optional<Intersection> intr = scene.intersect(orig, dir);
    if (!intr) return false;

    Vec3f& hit = intr->hit;
    Vec3f& n = intr->n;
    PhongMaterial& material =
        *(PhongMaterial*)(alloc.getMaterial(intr->object));

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

    for (int i = 0; i < alloc.lightsSize; ++i) {
      const SceneObject* light = &alloc.objects[alloc.lights[i]];
      const Mesh* lightMesh = alloc.getMesh(light);
      Vec3f dirLight = (lightMesh->pos() - hit).normalize();
      float NoL = dot(n, dirLight);

      // Casting shadow ray
      Vec3f shadowOrigin = NoL < 0 ? hit - n * 1e-3 : hit + n * 1e-3;
      if (auto intr = scene.intersect(shadowOrigin, dirLight)) {
        float dHit = (intr->hit - shadowOrigin).norm();
        float dLight = (lightMesh->pos() - shadowOrigin).norm();
        if (dLight - dHit > 1e-3) {
          continue;
        }
      }

      Vec3f L = Light(light).L();

      diffColor = diffColor + std::fmaxf(0.f, NoL) * L;

      specColor =
          specColor + std::powf(std::fmaxf(0.f, dot(reflectionDir, dirLight)),
                                material.specularExponent) *
                          L;
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
class RecursivePathtracer {
 public:
  int maxDepth = 8;

  Vec3f trace(const Scene& scene, Vec3f orig, Vec3f dir) {
    return traceBack(orig, dir, scene);
  }

  Vec3f traceBack(const Vec3f& orig, const Vec3f dir, const Scene& scene,
                  int depth = 0) {
    if (depth > maxDepth) return Vec3f(0);

    const Allocator& alloc = scene.allocator;

    compat::optional<Intersection> intr = scene.intersect(orig, dir);
    if (!intr) return Vec3f(0);

    Vec3f& hit = intr->hit;
    const Material& bsdf = *alloc.getMaterial(intr->object);
    ONB onb = ONB(intr->n);           // orthonormal basis with surface normal
    Ray ro = onb.toLocalSpace(-dir);  // view ray "outgoing ray"
    Ray ri;                           // light ray "incoming ray"
    Vec3f wiW;                        // light ray, world space

    if (intr->light) return ro.z > 0 ? intr->light->L() : 0;

    ri = bsdf.sampleRi(ro);

    if (IsSpecular(ri.flag)) {
      compat::optional<BSDFSample> bs = bsdf.sample(ro, ri);
      if (!bs) return Vec3f(0);
      Vec3f wiW = onb.toWorldSpace(ri);
      Vec3f Li = traceBack(hit + wiW * 1e-3, wiW, scene, depth + 1);
      return bs->fValue * Li;
    }

    bool isLightSampling = Sampler::Get1D() < .5;
    const auto& light = scene.sampleLight();

    if (isLightSampling && light) {
      wiW = alloc.getMesh(light)->generate(hit);
      ri = Ray(onb.toLocalSpace(wiW));
    } else {
      wiW = onb.toWorldSpace(ri);
    }

    compat::optional<BSDFSample> bs = bsdf.sample(ro, ri);
    if (!bs) return Vec3f(0);
    float NoL = ri.z;
    if (NoL < 0) return Vec3f(0);

    float lightPdf = light ? alloc.getMesh(light)->pdf(wiW, hit) : 0;
    float pdf = (lightPdf + bs->pdf) / 2;
    if (pdf < 1e-6) return Vec3f(0);

    Vec3f Li = traceBack(hit + wiW * 1e-3, wiW, scene, depth + 1);
    return bs->fValue * NoL * Li / pdf;
  }
};

/**
 * An iterative pathtracer using Monte Carlo & Direct Light Sampling,
 * based on `SimplePathtracer` from `PBRT`.
 */
class SimplePathtracer {
 public:
  int maxDepth = 8;

  Vec3f trace(const Scene& scene, Vec3f orig, Vec3f dir) {
    int depth = 0;                 // trace depth
    bool isSpecularBounce = true;  // initially true for direct light hit
    Ray ro;                        // view ray "outgoing ray"
    Ray ri;                        // light ray "incoming ray"
    Vec3f woW(-dir);               // view vector, world space
    Vec3f Lo(0);                   // radiance output
    Vec3f beta(1);                 // path throughput weight, "contribution"

    while (beta.x > 0 || beta.y > 0 || beta.z > 0) {
      if (depth++ == maxDepth) break;

      compat::optional<Intersection> intr = scene.intersect(orig, -woW);
      if (!intr) break;

      Vec3f& hit = intr->hit;
      const Material& bsdf = *scene.allocator.getMaterial(intr->object);
      ONB onb = ONB(intr->n);
      ro = onb.toLocalSpace(woW);

      if (intr->light) {
        if (isSpecularBounce && ro.z > 0) {
          Lo += beta * intr->light->L();
        }
        // Skip this hit
        orig = hit - woW * 1e-3;
        continue;
      }

      // Direct light sampling
      if (compat::optional<LightSample> ls = sampleDirectLight(scene, hit)) {
        Vec3f wl = onb.toLocalSpace(ls->dir);
        if (wl.z > 0) {
          Lo += beta * Light(ls->light).L() * bsdf.f(ro, wl) * std::abs(wl.z) /
                ls->pdf;
        }
      }

      // BSDF
      ri = bsdf.sampleRi(ro);
      isSpecularBounce = IsSpecular(ri.flag);

      compat::optional<BSDFSample> bs = bsdf.sample(ro, ri);
      if (!bs) break;
      float NoL = ri.z;

      if (isSpecularBounce) {
        beta *= bs->fValue;
      } else {
        if (NoL < 0 || bs->pdf < 1e-6) break;
        beta *= bs->fValue * NoL / bs->pdf;
      }

      woW = -onb.toWorldSpace(ri);
      orig = hit - woW * 1e-3;
    }

    return Lo;
  }

 private:
  struct LightSample {
    float pdf;
    Vec3f dir;
    const SceneObject* light;
  };

  compat::optional<LightSample> sampleDirectLight(const Scene& scene,
                                                  const Vec3f& pos) const {
    const SceneObject* light = scene.sampleLight();
    if (!light) return {};

    const Allocator& alloc = scene.allocator;

    const Mesh* mesh = alloc.getMesh(light);
    Vec3f wlW = mesh->generate(pos);

    compat::optional<Intersection> intr =
        scene.intersect(pos + wlW * 1e-3, wlW);
    bool occuluded = !intr || intr->object != light;
    if (occuluded) return {};

    float lightPdf = mesh->pdf(wlW, pos);
    if (lightPdf < 1e-6) return {};

    return LightSample{.pdf = lightPdf, .dir = wlW, .light = light};
  }
};

class Tracer {
  using TracerVariant =
      compat::variant<WhittedRaytracer, RecursivePathtracer, SimplePathtracer>;
  TracerVariant tracer;

 public:
  enum class Type { Whitted, Recursive, Simple };

  Tracer() : tracer(SimplePathtracer()) {}
  Tracer(TracerVariant tracer) : tracer(tracer) {}

  Tracer static Create(Type type) {
    switch (type) {
      case Type::Whitted:
        return Tracer{WhittedRaytracer()};
      case Type::Recursive:
        return Tracer{RecursivePathtracer()};
      case Type::Simple:
        return Tracer{SimplePathtracer()};
      default:
        return Tracer{SimplePathtracer()};
    }
  }

  Vec3f trace(const Scene* scene, Vec3f orig, Vec3f dir) {
    return compat::visit(
        [scene, orig, dir](auto&& tracer) -> Vec3f {
          return tracer.trace(*scene, orig, dir);
        },
        tracer);
  }
};
