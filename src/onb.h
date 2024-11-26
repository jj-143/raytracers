#pragma once

#include "math.h"

// Orthonormal basis
class ONB {
 public:
  ONB(Vec3f& n) {
    axis[2] = n.normalize();
    Vec3f a = (std::fabs(axis[2].x) > 0.9) ? Vec3f(0, 1, 0) : Vec3f(1, 0, 0);
    axis[1] = cross(axis[2], a).normalize();
    axis[0] = cross(axis[2], axis[1]);
  }

  const Vec3f& u() const { return axis[0]; }
  const Vec3f& v() const { return axis[1]; }
  const Vec3f& w() const { return axis[2]; }

  inline Vec3f toWorldSpace(const Vec3f& v) const {
    return (v[0] * axis[0]) + (v[1] * axis[1]) + (v[2] * axis[2]);
  }

  inline Vec3f toLocalSpace(const Vec3f& vW) const {
    return {dot(vW, axis[0]), dot(vW, axis[1]), dot(vW, axis[2])};
  }

 private:
  Vec3f axis[3];
};
