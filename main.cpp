#include <cmath>
#include <limits>
#include <vector>

#include "geometry.h"
#include "utils.h"

struct Sphere {
  Vec3f pos;
  int radius;
};

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
}

int main() {
  render();
  return 0;
}