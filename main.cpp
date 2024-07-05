/**
 * NOTE:
 * Framebuffer origin (0, 0) at top-left
 * Right (+X), Down (+Y), Facing Out from Screen (+Z)
 */

#include <cmath>
#include <iostream>
#include <limits>
#include <vector>

#include "geometry.h"
#include "utils.h"

struct Sphere {
  Vec3f pos;
  float radius;

  bool ray_intersect(const Vec3f &orig, const Vec3f &dir, float &t0) const {
    Vec3f L = (pos - orig);
    float tca = L * dir;
    float d2 = L * L - tca * tca;
    if (d2 > radius * radius) return false;
    float tcc = sqrtf(radius * radius - d2);  // bottom len of triangle
    t0 = tca - tcc;
    float t1 = tca + tcc;
    if (t0 < 0) {
      t0 = t1;
    }
    if (t0 < 0) return false;
    return true;
  }
};

void render() {
  const int width = 1024;
  const int height = 768;

  // cam
  float fov = 50 * M_PI / 180;

  std::vector<Vec3f> framebuffer(width * height);
  std::vector<float> depthbuffer(width * height);

  const Sphere sphere = {Vec3f(.3, .2, -2), 0.2};

  float min = 10000;
  float max = 0;

  // Depth Pass
  for (size_t j = 0; j < height; j++) {
    for (size_t i = 0; i < width; i++) {
      float x = (2 * (i + 0.5) / (float)width - 1) * tan(fov / 2.);
      float y = (2 * (j + 0.5) / (float)height - 1) * tan(fov / 2.) * height /
                (float)width;

      Vec3f dir = Vec3f(x, y, -1).normalize();

      float t0 = 0;
      bool hit = sphere.ray_intersect(Vec3f(0, 0, 0), dir, t0);
      if (hit) {
        depthbuffer[i + j * width] = t0;
        min = std::min(min, t0);
        max = std::max(max, t0);
      }
    }
  }

  // Normalize Depth Pass & Draw
  for (size_t j = 0; j < height; j++) {
    for (size_t i = 0; i < width; i++) {
      int idx = i + j * width;
      if (depthbuffer[idx] > 0) {
        float val = (depthbuffer[idx] - min) / (max - min);
        framebuffer[idx] = Vec3f(1 - val, 1 - val, 1 - val);
      }
    }
  }

  save(framebuffer, width, height);
}

int main() {
  render();
  return 0;
}