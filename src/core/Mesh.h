#pragma once

#include <string>

#include "../math.h"
#include "Sampler.h"

class Mesh {
 public:
  std::string name;
  Vec3f pos;

  virtual inline bool intersect(const Vec3f &orig, const Vec3f &dir, float &t0,
                                Vec3f &N) const {
    return false;
  }

  Mesh(const Vec3f pos, const std::string name = "") : pos(pos), name(name) {}
};

class Samplable {
 public:
  virtual inline float pdf(const Vec3f &direction, const Vec3f &origin) const {
    return 0;
  }

  virtual inline Vec3f generate(const Vec3f &origin) const {
    return Vec3f(1, 0, 0);
  }
};

class SamplableMesh : public Mesh, public Samplable {
  using Mesh::Mesh;
};

class Sphere : public Mesh {
 public:
  float radius;

  inline bool intersect(const Vec3f &orig, const Vec3f &dir, float &t0,
                        Vec3f &N) const override {
    Vec3f L = (pos - orig);
    float tca = dot(L, dir);
    float d2 = dot(L, L) - tca * tca;
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

class Plane : public SamplableMesh {
 public:
  float halfWidth;
  float halfHeight;
  Vec3f normal = Vec3f(0, 1, 0);

  inline bool intersect(const Vec3f &orig, const Vec3f &dir, float &t0,
                        Vec3f &N) const override {
    Vec3f L = (pos - orig);
    float dirDotNormal = dot(dir, normal);
    if (std::fabs(dirDotNormal) < 1e-8) {
      return false;
    }
    float d = dot(L, normal) * (1 / dirDotNormal);
    Vec3f r = dir * d - L;

    // split components
    float w = dot(r, dirW);
    float h = dot(r, dirH);

    if (-halfWidth < w && w < halfWidth && -halfHeight < h && h < halfHeight) {
      t0 = d;
      if (t0 < 0) return false;
      N = normal;
      return true;
    } else {
      return false;
    }
  }

  inline float pdf(const Vec3f &direction, const Vec3f &origin) const override {
    float d;
    Vec3f N;

    if (!intersect(origin, direction, d, N)) return 0;
    float NoL = -dot(N, direction);
    if (NoL < 1e-6) return 0;

    float lightArea = halfWidth * halfHeight * 4;
    return d * d / (NoL * lightArea);
  }

  Vec3f generate(const Vec3f &origin) const override {
    Vec2f u = Sampler::Get2D();
    Vec3f p{pos.x + u.x * 2 * halfWidth - halfWidth, pos.y,
            pos.z + u.y * 2 * halfHeight - halfHeight};
    return (p - origin).normalize();
  }

  Plane(Vec3f p, float w, float h, Vec3f normal,
        const std::string name = "Plane")
      : SamplableMesh(p, name), halfWidth(w), halfHeight(h), normal(normal) {
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
