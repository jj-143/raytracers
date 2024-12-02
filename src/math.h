#pragma once

#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

// Random
inline float randf() { return rand() / (RAND_MAX + 1.f); }

inline float randf(float low, float high) {
  float v = rand() / (RAND_MAX + 1.0);
  return low + v * (high - low);
}

static unsigned int gSeed;

// Used to seed the generator.
inline void sFastRand(int seed) { gSeed = seed; }

// Compute a pseudorandom integer.
// Output value in range [0, 32767]
inline int fastRand() {
  gSeed = (214013 * gSeed + 2531011);
  return (gSeed >> 16) & 0x7FFF;
}

template <typename T>
inline T _rand() {
  return static_cast<T>(rand()) / static_cast<T>(RAND_MAX);
}

template <size_t DIM, typename T>
struct vec {
  vec() { for (size_t i = DIM; i--; data_[i] = T()); }
  T& operator[](const size_t i) {
    assert(i < DIM);
    return data_[i];
  }
  const T& operator[](const size_t i) const {
    assert(i < DIM);
    return data_[i];
  }

 private:
  T data_[DIM];
};

typedef vec<2, float> Vec2f;
typedef vec<3, float> Vec3f;
typedef vec<3, int> Vec3i;
typedef vec<4, float> Vec4f;

template <typename T>
struct vec<2, T> {
  vec() : x(T()), y(T()) {}
  vec(T X, T Y) : x(X), y(Y) {}
  template <class U>
  vec(const vec<2, U>& v);
  T& operator[](const size_t i) {
    assert(i < 2);
    return i <= 0 ? x : y;
  }
  const T& operator[](const size_t i) const {
    assert(i < 2);
    return i <= 0 ? x : y;
  }
  T x, y;
};

template <typename T>
struct vec<3, T> {
  vec() : x(T()), y(T()), z(T()) {}
  vec(T X, T Y, T Z) : x(X), y(Y), z(Z) {}
  vec(T V) : x(V), y(V), z(V) {}
  T& operator[](const size_t i) {
    assert(i < 3);
    return i <= 0 ? x : (1 == i ? y : z);
  }
  const T& operator[](const size_t i) const {
    assert(i < 3);
    return i <= 0 ? x : (1 == i ? y : z);
  }
  float norm() { return std::sqrt(x * x + y * y + z * z); }
  vec<3, T>& normalize(T l = 1) {
    *this = (*this) * (l / norm());
    return *this;
  }
  inline static vec<3, T> rand() {
    return vec<3, T>(_rand<T>(), _rand<T>(), _rand<T>());
  }
  inline static vec<3, T> rand(T min, T max) {
    return vec<3, T>(_rand<T>() * (max - min) + min,
                     _rand<T>() * (max - min) + min,
                     _rand<T>() * (max - min) + min);
  }
  inline static vec<3, T> rand_unit() {
    while (true) {
      auto p = vec<3, T>::rand(-1, 1);
      auto length_squared = p.x * p.x + p.y * p.y + p.z * p.z;
      if (length_squared <= 1) {
        auto len = std::sqrt(length_squared);
        return vec<3, T>(p.x / len, p.y / len, p.z / len);
      }
    }
  }
  inline static vec<3, T> rand_unit(const vec<3, T>& dir) {
    vec<3, T> p = vec<3, T>::rand_unit();
    return p * dir > 0 ? p : -p;
  }
  T x, y, z;
};

template <typename T>
struct vec<4, T> {
  vec() : x(T()), y(T()), z(T()), w(T()) {}
  vec(T X, T Y, T Z, T W) : x(X), y(Y), z(Z), w(W) {}
  T& operator[](const size_t i) {
    assert(i < 4);
    return i <= 0 ? x : (1 == i ? y : (2 == i ? z : w));
  }
  const T& operator[](const size_t i) const {
    assert(i < 4);
    return i <= 0 ? x : (1 == i ? y : (2 == i ? z : w));
  }
  T x, y, z, w;
};

template <size_t DIM, typename T>
vec<DIM, T> operator*(const vec<DIM, T>& lhs, const vec<DIM, T>& rhs) {
  vec<DIM, T> ret;
  for (size_t i = DIM; i--; ret[i] = lhs[i] * rhs[i]);
  return ret;
}

template <size_t DIM, typename T>
T dot(const vec<DIM, T>& lhs, const vec<DIM, T>& rhs) {
  T ret = T();
  for (size_t i = DIM; i--; ret += lhs[i] * rhs[i]);
  return ret;
}

template <size_t DIM, typename T>
vec<DIM, T> operator+(vec<DIM, T> lhs, const vec<DIM, T>& rhs) {
  for (size_t i = DIM; i--; lhs[i] += rhs[i]);
  return lhs;
}

template <size_t DIM, typename T>
vec<DIM, T> operator-(vec<DIM, T> lhs, const vec<DIM, T>& rhs) {
  for (size_t i = DIM; i--; lhs[i] -= rhs[i]);
  return lhs;
}

template <size_t DIM, typename T, typename U>
vec<DIM, T> operator*(const vec<DIM, T>& lhs, const U& rhs) {
  vec<DIM, T> ret;
  for (size_t i = DIM; i--; ret[i] = lhs[i] * rhs);
  return ret;
}

template <size_t DIM, typename T, typename U>
vec<DIM, T> operator*(const U& lhs, const vec<DIM, T>& rhs) {
  vec<DIM, T> ret;
  for (size_t i = DIM; i--; ret[i] = rhs[i] * lhs);
  return ret;
}

template <size_t DIM, typename T, typename U>
vec<DIM, T> operator/(const vec<DIM, T>& lhs, const U& rhs) {
  vec<DIM, T> ret;
  for (size_t i = DIM; i--; ret[i] = lhs[i] / rhs);
  return ret;
}

template <size_t DIM, typename T>
vec<DIM, T> operator-(const vec<DIM, T>& lhs) {
  return lhs * T(-1);
}

template <typename T>
vec<3, T> cross(vec<3, T> v1, vec<3, T> v2) {
  return vec<3, T>(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z,
                   v1.x * v2.y - v1.y * v2.x);
}

template <size_t DIM, typename T>
std::ostream& operator<<(std::ostream& out, const vec<DIM, T>& v) {
  for (unsigned int i = 0; i < DIM; i++) {
    out << v[i] << " ";
  }
  return out;
}

inline bool refract(const Vec3f& d, const Vec3f& N, float eta, Vec3f& dir) {
  float cosI = -dot(d, N);
  float sinI = sqrtf(1 - cosI * cosI);
  if (sinI > 1 / eta) return false;  // total reflection

  float sinT = eta * sinI;
  float cosT = sqrtf(1 - sinT * sinT);

  Vec3f dH = (d + N * cosI).normalize();
  Vec3f dN = -N;

  dir = dH * sinT + dN * cosT;
  return true;
}
