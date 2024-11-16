#pragma once
#include <memory>
#include <string>

#include "math.h"

class Mesh {
 public:
  std::string name;
  Vec3f pos;

  virtual bool rayIntersect(const Vec3f &orig, const Vec3f &dir, float &t0,
                            Vec3f &N) const {
    return false;
  }

  Mesh(const Vec3f pos, const std::string name = "") : pos(pos), name(name) {}
};

class Sphere : public Mesh {
 public:
  float radius;

  bool rayIntersect(const Vec3f &orig, const Vec3f &dir, float &t0,
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

  Sphere(const Vec3f p, const float r, const std::string name = "Sphere")
      : Mesh(p, name), radius(r) {}
};

class Plane : public Mesh {
 public:
  float halfWidth;
  float halfHeight;
  Vec3f normal = Vec3f(0, 1, 0);

  bool rayIntersect(const Vec3f &orig, const Vec3f &dir, float &t0,
                    Vec3f &N) const override {
    Vec3f L = (pos - orig);
    float d = L * normal * (1 / (dir * normal));
    Vec3f r = dir * d - L;

    // split components
    float w = r * dirW;
    float h = r * dirH;

    if (-halfWidth < w && w < halfWidth && -halfHeight < h && h < halfHeight) {
      t0 = d;
      if (t0 < 0) return false;
      N = normal;
      return true;
    } else {
      return false;
    }
  }

  Plane(Vec3f p, float w, float h, Vec3f normal,
        const std::string name = "Plane")
      : Mesh(p, name), halfWidth(w), halfHeight(h), normal(normal) {
    this->normal = normal.normalize();
    Vec3f up = {0, 1, 0};
    Vec3f right = cross(up, this->normal);

    // if normal is almost +Y/-Y: use {1, 0, 0} as dirW
    // else: use right as dirW
    if (right.norm() != 0) {
      dirW = right.normalize();
    }
    dirH = cross(this->normal, dirW);
  }

 private:
  Vec3f dirW = Vec3f(1, 0, 0);
  Vec3f dirH = Vec3f(0, 0, -1);
};

struct Material {
  Vec3f diffuseColor;
  std::vector<float> albedo = {1, 0, 0, 0};
  float specularExponent;
  float refractiveIndex = 1;
};

class Object {
 public:
  std::shared_ptr<Mesh> target;
  std::shared_ptr<Material> material;

  Object() = default;
  Object(std::shared_ptr<Mesh> m, std::shared_ptr<Material> mat)
      : target(m), material(mat) {}
};

struct PointLight {
  Vec3f pos;
  float intensity;
};
