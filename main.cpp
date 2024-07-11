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

class Mesh {
 public:
  Vec3f pos;
  virtual bool ray_intersect(const Vec3f &orig, const Vec3f &dir, float &t0,
                             Vec3f &N) const {
    return false;
  }

  Mesh(const Vec3f pos) : pos(pos) {}
};

class Sphere : public Mesh {
 public:
  float radius;

  bool ray_intersect(const Vec3f &orig, const Vec3f &dir, float &t0,
                     Vec3f &N) const override {
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

    N = (dir * t0 - pos).normalize();
    return true;
  }

  Sphere(const Vec3f p, const float r) : Mesh(p), radius(r) {}
};

class Plane : public Mesh {
 public:
  float half_width;
  float half_height;
  Vec3f rotation;  // currently, only X rot is allowed.
  Vec3f face_normal = Vec3f(0, -1, 0);

  bool ray_intersect(const Vec3f &orig, const Vec3f &dir, float &t0,
                     Vec3f &N) const override {
    Vec3f normal = Vec3f(0, -1 * cos(rotation.x), sin(rotation.x));
    Vec3f L = (pos - orig);
    float d = L * normal * (1 / (dir * normal));
    Vec3f r = dir * d - L;

    // split component
    Vec3f dir_w = Vec3f(1, 0, 0);
    Vec3f dir_h = Vec3f(0, sin(rotation.x), cos(rotation.x));
    float w = r * dir_w;
    float h = r * dir_h;

    if (-half_width < w && w < half_width && -half_height < h &&
        h < half_height) {
      t0 = d;
      if (t0 < 0) return false;
      N = normal;
      return true;
    } else {
      return false;
    }
  }

  Plane(Vec3f p, float w, float h, Vec3f rot)
      : Mesh(p), half_width(w), half_height(h), rotation(rot) {}
};

class Object {
 public:
  Mesh *target;
  Vec3f color;

  Object() = default;
  Object(Mesh &m, const Vec3f c) : target(&m), color(c) {}
};

struct PointLight {
  Vec3f pos;
  float intensity;
};

bool scene_intersect(const Vec3f &origin, const Vec3f &dir,
                     const std::vector<Object> &objects, Vec3f &hit, Vec3f &N,
                     Object &object) {
  float t0 = 0;
  for (size_t i = 0; i < objects.size(); i++) {
    Mesh *mesh = objects[i].target;
    if (mesh->ray_intersect(origin, dir, t0, N)) {
      hit = dir * t0 + origin;
      object = objects[i];
      return true;
    }
  }
  return false;
}

bool cast_ray(const Vec3f &orig, const Vec3f dir, Vec3f &color,
              const std::vector<Object> &objects,
              const std::vector<PointLight> &lights) {
  Vec3f hit = Vec3f(0, 0, 0);
  Vec3f n = Vec3f(0, 0, 0);
  Object object;

  if (!scene_intersect(Vec3f(0, 0, 0), dir, objects, hit, n, object)) {
    return false;
  };

  float diff_intensity = 0;
  float spec_intensity = 0;
  Vec3f r = (dir + n * ((dir * n) * -2));

  for (size_t light_i = 0; light_i < lights.size(); light_i++) {
    const PointLight light = lights[light_i];
    Vec3f dir_light = (light.pos - hit).normalize();

    // shadow
    Vec3f temp_hit = Vec3f(0, 0, 0);
    Vec3f temp_n = Vec3f(0, 0, 0);
    Object temp_object;
    Vec3f shadow_origin = dir_light * n < 0 ? hit - n * 1e-3 : hit + n * 1e-3;
    if (scene_intersect(shadow_origin, dir_light, objects, temp_hit, temp_n,
                        temp_object))
      continue;

    diff_intensity += std::max(0.f, n * dir_light) * light.intensity;

    spec_intensity += powf(std::max(0.f, r * dir_light), 50) * light.intensity;
  }

  color = (object.color * (diff_intensity + spec_intensity));
  return true;
}

void render() {
  const int width = 1024;
  const int height = 768;

  // cam
  float fov = 50 * M_PI / 180;

  std::vector<Vec3f> framebuffer(width * height);
  std::vector<float> depthbuffer(width * height);

  const Vec3f BACKGROUND_COLOR = Vec3f(.9, .9, .9);

  std::vector<PointLight> lights;
  lights.push_back({Vec3f(0, -1.2, -0.5), 0.8});
  lights.push_back({Vec3f(.2, -1.6, -0.5), 0.3});
  lights.push_back({Vec3f(-.2, -1.4, -0.5), 0.3});

  std::vector<Object> objects;

  Sphere s1 = {Vec3f(.2, .2, -2.3), 0.2};
  Sphere s2 = {Vec3f(-.2, -.1, -2), 0.25};
  Sphere s3 = {Vec3f(.4, -.3, -2.3), 0.3};

  Plane plane = {Vec3f(0, .3, -3), .8, .8, Vec3f(M_PI / 6, 0, 0)};

  objects.push_back({s1, Vec3f(.8, .2, .2)});
  objects.push_back({s2, Vec3f(.2, .8, .2)});
  objects.push_back({s3, Vec3f(.2, .2, .8)});
  objects.push_back({plane, Vec3f(1, 1, 1)});

  // Render Pass
  for (size_t j = 0; j < height; j++) {
    for (size_t i = 0; i < width; i++) {
      float x = (2 * (i + 0.5) / (float)width - 1) * tan(fov / 2.);
      float y = (2 * (j + 0.5) / (float)height - 1) * tan(fov / 2.) * height /
                (float)width;
      Vec3f dir = Vec3f(x, y, -1).normalize();
      Vec3f color = Vec3f();
      if (!cast_ray(Vec3f(0, 0, 0), dir, color, objects, lights)) {
        color = BACKGROUND_COLOR;
      }
      framebuffer[i + j * width] = color;
    }
  }

  save(framebuffer, width, height);
}

int main() {
  render();
  return 0;
}