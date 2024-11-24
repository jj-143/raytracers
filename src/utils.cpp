#include <chrono>
#include <fstream>
#include <iostream>
#include <vector>

#include "math.h"

float gammaTransform(float value, float gamma) {
  if (value > 0) {
    return std::powf(value, 1 / gamma);
  }
  return 0;
}

void save(std::vector<Vec3f> framebuffer, int width, int height,
          std::string filename) {
  std::ofstream ofs;  // save the framebuffer to file
  std::string filepath = filename + ".ppm";
  ofs.open(filepath);

  ofs << "P6\n" << width << " " << height << "\n255\n";
  for (size_t i = 0; i < height * width; ++i) {
    for (size_t j = 0; j < 3; j++) {
      ofs << (char)(255 * std::max(0.f, std::min(1.f, framebuffer[i][j])));
    }
  }
  ofs.close();
}

int64_t nowMillis() {
  auto epoch = std::chrono::system_clock::now().time_since_epoch();
  auto millis =
      std::chrono::duration_cast<std::chrono::milliseconds>(epoch).count();
  return millis;
}