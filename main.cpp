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

  Sphere(const Vec3f p, float r) : pos(p), radius(r) {}

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

struct PointLight {
  Vec3f pos;
  float intensity;
};

struct Object {
  Sphere target;
  Vec3f color;
};

bool scene_intersect(const Vec3f &origin, const Vec3f &dir,
                     const std::vector<Object> &objects, Vec3f &hit, Vec3f &N,
                     Object &object) {
  float t0 = 0;
  for (size_t i = 0; i < 3; i++) {
    const Sphere sphere = objects[i].target;
    if (sphere.ray_intersect(origin, dir, t0)) {
      hit = dir * t0 + origin;
      N = (hit - sphere.pos).normalize();
      object = objects[i];
      return true;
    }
  }
  return false;
}

void render() {
  const int width = 1024;
  const int height = 768;

  // cam
  float fov = 50 * M_PI / 180;

  std::vector<Vec3f> framebuffer(width * height);
  std::vector<float> depthbuffer(width * height);

  std::vector<PointLight> lights;
  lights.push_back({Vec3f(0, -1.2, -0.5), 0.8});
  lights.push_back({Vec3f(.2, -1.6, -0.5), 0.3});
  lights.push_back({Vec3f(-.2, -1.4, -0.5), 0.3});

  std::vector<Object> objects;
  objects.push_back({Sphere(Vec3f(.2, .2, -2.3), 0.2), Vec3f(.8, .2, .2)});
  objects.push_back({Sphere(Vec3f(-.2, -.1, -2), 0.25), Vec3f(.2, .8, .2)});
  objects.push_back({Sphere(Vec3f(.1, -.3, -1.7), 0.1), Vec3f(.2, .2, .8)});
  const float material_specular = 50;

  float min = 10000;
  float max = 0;

  // Render Pass
  for (size_t j = 0; j < height; j++) {
    for (size_t i = 0; i < width; i++) {
      float x = (2 * (i + 0.5) / (float)width - 1) * tan(fov / 2.);
      float y = (2 * (j + 0.5) / (float)height - 1) * tan(fov / 2.) * height /
                (float)width;
      Vec3f dir = Vec3f(x, y, -1).normalize();
      Vec3f hit = Vec3f(0, 0, 0);
      Vec3f n = Vec3f(0, 0, 0);
      Object object = objects[0];  // TODO: make it null
      if (!scene_intersect(Vec3f(0, 0, 0), dir, objects, hit, n, object))
        continue;

      // Store depth
      const float depth = hit.norm();
      depthbuffer[i + j * width] = depth;
      min = std::min(min, depth);
      max = std::max(max, depth);

      float diff_intensity = 0;
      float spec_intensity = 0;

      for (size_t light_i = 0; light_i < lights.size(); light_i++) {
        const PointLight light = lights[light_i];
        Vec3f dir_light = (light.pos - hit).normalize();

        // shadow
        Vec3f temp_hit = Vec3f(0, 0, 0);
        Vec3f temp_n = Vec3f(0, 0, 0);
        Object temp_object = objects[0];
        Vec3f shadow_origin =
            dir_light * n < 0 ? hit - n * 1e-3 : hit + n * 1e-3;
        if (scene_intersect(shadow_origin, dir_light, objects, temp_hit, temp_n,
                            temp_object))
          continue;

        diff_intensity += std::max(0.f, n * dir_light) * light.intensity;

        Vec3f r = (dir + n * ((dir * n) * -2));

        spec_intensity +=
            powf(std::max(0.f, r * dir_light), material_specular) *
            light.intensity;
      }
      framebuffer[i + j * width] =
          object.color * (diff_intensity + spec_intensity);
    }
  }

  save(framebuffer, width, height);
}

int main() {
  render();
  return 0;
}