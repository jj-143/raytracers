#pragma once
#include "geometry.h"

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

    N = (dir * t0 + orig - pos).normalize();
    return true;
  }

  Sphere(const Vec3f p, const float r) : Mesh(p), radius(r) {}
};

class Plane : public Mesh {
 public:
  float half_width;
  float half_height;
  Vec3f rotation;  // currently, only X rot is allowed.
  Vec3f face_normal = Vec3f(0, 1, 0);

  bool ray_intersect(const Vec3f &orig, const Vec3f &dir, float &t0,
                     Vec3f &N) const override {
    Vec3f normal = Vec3f(0, cos(rotation.x), sin(rotation.x));
    Vec3f L = (pos - orig);
    float d = L * normal * (1 / (dir * normal));
    Vec3f r = dir * d - L;

    // split component
    Vec3f dir_w = Vec3f(1, 0, 0);
    Vec3f dir_h = Vec3f(0, -sin(rotation.x), cos(rotation.x));
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

struct Material {
  Vec3f diffuse_color;
  Vec3f albedo = Vec3f(1, 0, 0);
  float specular_exponent;
};

class Object {
 public:
  Mesh *target;
  Material material;

  Object() = default;
  Object(Mesh &m, Material mat) : target(&m), material(mat) {}
};

struct PointLight {
  Vec3f pos;
  float intensity;
};
