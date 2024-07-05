#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <vector>

#include "geometry.h"

struct Sphere {
  Vec3f pos;
  int radius;
};

void save(std::vector<Vec3f> framebuffer, int width, int height) {
  std::ofstream ofs;  // save the framebuffer to file
  ofs.open("./build/out.ppm");
  ofs << "P6\n" << width << " " << height << "\n255\n";
  for (size_t i = 0; i < height * width; ++i) {
    for (size_t j = 0; j < 3; j++) {
      ofs << (char)(255 * std::max(0.f, std::min(1.f, framebuffer[i][j])));
    }
  }
  ofs.close();
}

void render() {
  const int width = 1024;
  const int height = 768;
  std::vector<Vec3f> framebuffer(width * height);

  const Sphere sphere = {Vec3f(100, 200, 100), 50};

  for (size_t j = 0; j < height; j++) {
    for (size_t i = 0; i < width; i++) {
      framebuffer[i + j * width] =
          Vec3f(j / float(height), i / float(width), 0);
    }
  }

  for (size_t j = 0; j < height; j++) {
    for (size_t i = 0; i < width; i++) {
      const int ds = (j - sphere.pos[0]) * (j - sphere.pos[0]) +
                     (i - sphere.pos[1]) * (i - sphere.pos[1]);

      if (ds <= sphere.radius * sphere.radius) {
        framebuffer[i + j * width] = Vec3f(0.5f, 0.5f, 0.5f);
      }
    }
  }


  save(framebuffer, width, height);
  // std::ofstream ofs;  // save the framebuffer to file
  // ofs.open("./build/out.ppm");
  // ofs << "P6\n" << width << " " << height << "\n255\n";
  // for (size_t i = 0; i < height * width; ++i) {
  //   for (size_t j = 0; j < 3; j++) {
  //     ofs << (char)(255 * std::max(0.f, std::min(1.f, framebuffer[i][j])));
  //   }
  // }
  // ofs.close();
}

int main() {
  render();
  return 0;
}