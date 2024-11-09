/**
 * NOTE:
 * Framebuffer origin (0, 0) at top-left.
 * Screen space origin (0, 0) at bottom-left:
 *  - Right (+X), Up (+Y), Facing Out from Screen (+Z)
 */

#include <cmath>
#include <iostream>
#include <limits>
#include <vector>

#include "common.h"
#include "geometry.h"
#include "utils.h"

/**
 * Return a random float in [0, 1]
 */
inline float randf() {
  return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

const Vec3f BACKGROUND_COLOR = Vec3f(0, 0, 0);
const int MAX_REFLECTION_DEPTH = 4;
const int MAX_REFRACTION_DEPTH = 12;
const int N_SAMPLES = 4;

const float REFRACTIVE_INDEX_ENVIRONMENT = 1;  // air: 1, water: 1.33

bool sceneIntersect(const Vec3f &origin, const Vec3f &dir,
                    const std::vector<Object> &objects, Vec3f &hit, Vec3f &N,
                    Object &object) {
  float t0 = MAXFLOAT;
  for (size_t i = 0; i < objects.size(); i++) {
    Mesh *mesh = objects[i].target;
    float tempDist = 0;
    Vec3f tempN;
    if (mesh->rayIntersect(origin, dir, tempDist, tempN) && tempDist < t0) {
      t0 = tempDist;
      N = tempN;
      hit = dir * t0 + origin;
      object = objects[i];
    }
  }
  return t0 < MAXFLOAT;
}

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

bool castRay(const Vec3f &orig, const Vec3f dir, Vec3f &color,
             const std::vector<Object> &objects,
             const std::vector<PointLight> &lights, int depthReflection = 0,
             int depthRefraction = 0) {
  Vec3f hit = Vec3f(0, 0, 0);
  Vec3f n = Vec3f(0, 0, 0);
  Object object;

  // Also discard the ray if it exceeds maximum depth as if it doesn't hit
  // anything.
  // Alternative option is to not casting the [reflection | refraction] ray
  // while returning back only the diffuse and specular.
  if (depthReflection > MAX_REFLECTION_DEPTH ||
      depthRefraction > MAX_REFRACTION_DEPTH ||
      !sceneIntersect(orig, dir, objects, hit, n, object)) {
    return false;
  };

  float diffIntensity = 0;
  float specIntensity = 0;
  Vec3f r = (dir + n * ((dir * n) * -2));

  // reflection
  Vec3f reflectionColor = Vec3f(0, 0, 0);

  Vec3f reflectionDir = r.normalize();
  Vec3f reflectionOrigin = hit + r * 1e-3;

  if (!castRay(reflectionOrigin, reflectionDir, reflectionColor, objects,
               lights, depthReflection + 1, depthRefraction)) {
    reflectionColor = BACKGROUND_COLOR;
  };

  // Refraction
  Vec3f refractionColor = Vec3f(0, 0, 0);
  if (object.material.albedo[3] > 0) {
    bool isInside = (dir * n) > 0;
    float indexI = isInside ? object.material.refractiveIndex
                            : REFRACTIVE_INDEX_ENVIRONMENT;
    float indexT = isInside ? REFRACTIVE_INDEX_ENVIRONMENT
                            : object.material.refractiveIndex;

    Vec3f dirRefraction;
    if (refract(dir, isInside ? -n : n, indexI, indexT, dirRefraction)) {
      dirRefraction = dirRefraction.normalize();
      if (!castRay(hit + dirRefraction * 1e-3, dirRefraction, refractionColor,
                   objects, lights, depthReflection, depthRefraction + 1)) {
        refractionColor = BACKGROUND_COLOR;
      }
    }
  }

  for (size_t iLight = 0; iLight < lights.size(); iLight++) {
    const PointLight light = lights[iLight];
    Vec3f dirLight = (light.pos - hit).normalize();

    // shadow
    Vec3f tempHit = Vec3f(0, 0, 0);
    Vec3f tempN = Vec3f(0, 0, 0);
    Object tempObject;
    Vec3f shadowOrigin = dirLight * n < 0 ? hit - n * 1e-3 : hit + n * 1e-3;
    if (sceneIntersect(shadowOrigin, dirLight, objects, tempHit, tempN,
                       tempObject)) {
      float dist = (tempHit - shadowOrigin).norm();
      if (dist < (light.pos - hit).norm()) {
        continue;
      }
    }

    diffIntensity += std::max(0.f, n * dirLight) * light.intensity;

    specIntensity +=
        powf(std::max(0.f, r * dirLight), object.material.specularExponent) *
        light.intensity;
  }

  color =
      object.material.diffuseColor * diffIntensity * object.material.albedo[0] +
      Vec3f(1, 1, 1) * specIntensity * object.material.albedo[1] +
      reflectionColor * object.material.albedo[2] +
      refractionColor * object.material.albedo[3];
  return true;
}

void render(const int &width, const int &height) {
  // Camera
  Vec3f camOrig = {.275, .275, .8};

  // 35mm lens with 25mm sensor
  float fov = 39.3076 * M_PI / 180;
  float pixelSize = tan(fov / 2) / width;

  std::vector<Vec3f> framebuffer(width * height);
  std::vector<float> depthbuffer(width * height);

  std::vector<PointLight> lights;
  lights.push_back({Vec3f(.275, .548, -.275), 1.5});

  std::vector<Object> objects;

  // clang-format off
  Sphere s1        = {Vec3f(.425, .080, -.344), .08}; // red
  Sphere s2        = {Vec3f(.125, .080, -.344), .08}; // green
  Sphere s3        = {Vec3f(.195, .275, -.470), .08}; // yellow
  Sphere s4        = {Vec3f(.355, .275, -.470), .08}; // mirror
  Sphere s5        = {Vec3f(.275, .080, -.140), .08}; // glass
  Plane floor      = {Vec3f(.275,    0, -.275), .275, .275, Vec3f(0, 1, 0)};
  Plane ceiling    = {Vec3f(.275,  .55, -.275), .275, .275, Vec3f(0, -1, 0)};
  Plane wallLeft   = {Vec3f(   0, .275, -.275), .275, .275, Vec3f(1, 0, 0)};
  Plane wallRight  = {Vec3f( .55, .275, -.275), .275, .275, Vec3f(-1, 0, 0)};
  Plane wallBack   = {Vec3f(.275, .275,  -.55), .275, .275, Vec3f(0, 0, 1)};

  Material red        = Material{Vec3f(  1,  0,  0), {  .6,  .3,  .01,   0},  100,    1};
  Material green      = Material{Vec3f(  0,  1,  0), {  .6,  .3,  .01,   0},   20,    1};
  Material yellow     = Material{Vec3f(  1,  1,  0), {  .6,  .3,  .01,   0},  100,    1};
  Material white      = Material{Vec3f(  1,  1,  1), {  .6,  .1,  .01,   0},   10,    1};
  Material mirror     = Material{Vec3f(  1,  1,  1), {   0,  10,  .8,   0}, 1500,    1};
  Material glass      = Material{Vec3f( .6, .7, .8), {  .1,  10,  .01,  .8}, 1500,  1.5};
  // clang-format on

  objects.push_back({s1, red});
  objects.push_back({s2, green});
  objects.push_back({s3, yellow});
  objects.push_back({s4, mirror});
  objects.push_back({s5, glass});
  objects.push_back({floor, white});
  objects.push_back({ceiling, white});
  objects.push_back({wallLeft, red});
  objects.push_back({wallRight, green});
  objects.push_back({wallBack, white});

  int64_t now = nowMillis();

  // Render Pass
#pragma omp parallel for collapse(2)
  for (size_t j = 0; j < height; j++) {
    for (size_t i = 0; i < width; i++) {
      Vec3f color = Vec3f();

      for (size_t iSample = 0; iSample < N_SAMPLES; iSample++) {
        Vec3f sampleColor = Vec3f();
        float x = (2 * (i + 0.5) / (float)width - 1) * tan(fov / 2.);
        float y = -(2 * (j + 0.5) / (float)height - 1) * tan(fov / 2.) *
                  height / (float)width;

        // Random sample inside the pixel;
        x += (randf() - .5) * pixelSize;
        y += (randf() - .5) * pixelSize;

        Vec3f dir = Vec3f(x, y, -1).normalize();

        if (!castRay(camOrig, dir, sampleColor, objects, lights)) {
          sampleColor = BACKGROUND_COLOR;
        }
        color = color + sampleColor;
      }
      color = color * (1.f / N_SAMPLES);
      framebuffer[i + j * width] = color;
    }
  }

  std::cout << "Render took " << (nowMillis() - now) << "ms" << std::endl;

  save(framebuffer, width, height);
}

int main(int argc, char *argv[]) {
  const int width = argc > 1 ? atoi(argv[1]) : 512;
  const int height = argc > 2 ? atoi(argv[2]) : 512;

  printf("Frame size: (%d, %d)\n", width, height);

  if (!(width > 0 && height > 0)) {
    fprintf(stderr, "Error: invalid argument: [width height]\n");
    fprintf(stderr, "usage: main [width height]\n");
    return 2;
  }

  render(width, height);
  return 0;
}