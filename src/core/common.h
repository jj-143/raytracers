#pragma once

#include "math.h"
#include "raytracers.h"

class Ray : public Vec3f {
 public:
  enum Flag {
    Unset = 0,
    Reflection = 1 << 0,
    Transmission = 1 << 1,
    Diffuse = 1 << 2,
    Specular = 1 << 3,

    SpecularReflection = Specular | Reflection,
    SpecularTransmission = Specular | Transmission
  };

  Flag flag = Unset;

  RT_DEVICE_HOST Ray() {};
  RT_DEVICE_HOST Ray(Vec3f dir) : Vec3f(dir) {};
  RT_DEVICE_HOST Ray(Vec3f dir, Flag flag) : Vec3f(dir), flag(flag) {};
};

RT_DEVICE_HOST inline Ray::Flag operator|(Ray::Flag a, Ray::Flag b) {
  return Ray::Flag((int)a | (int)b);
}
RT_DEVICE_HOST inline Ray::Flag& operator|=(Ray::Flag& a, Ray::Flag b) {
  (int&)a |= (int)b;
  return a;
}
RT_DEVICE_HOST inline Ray::Flag operator&(Ray::Flag a, Ray::Flag b) {
  return Ray::Flag((int)a & (int)b);
}
RT_DEVICE_HOST inline bool IsTransmission(Ray::Flag f) {
  return int(f & Ray::Transmission);
}
RT_DEVICE_HOST inline bool IsReflection(Ray::Flag f) {
  return int(f & Ray::Reflection);
}
RT_DEVICE_HOST inline bool IsSpecular(Ray::Flag f) {
  return int(f & Ray::Specular);
}

struct BSDFSample {
  Vec3f fValue;
  float pdf;

  RT_DEVICE_HOST BSDFSample(Vec3f fValue, float pdf)
      : fValue(fValue), pdf(pdf) {}
};
