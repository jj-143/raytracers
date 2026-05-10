#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_write.h>

#include <chrono>
#include <fstream>
#include <iostream>
#include <vector>

#include "math.h"

namespace {
bool writeToPNG(const char* filepath, void* data, const int& width,
                const int& height, const int& channel) {
  const int result =
      stbi_write_png(filepath, width, height, channel, data, channel * width);
  return result != 0;  // 0 for Error
}
}  // namespace

float gammaTransform(float value, float gamma) {
  if (value > 0) {
    return std::powf(value, 1 / gamma);
  }
  return 0;
}

void saveToPPM(std::vector<Vec3f> framebuffer, int width, int height,
               std::string filepath) {
  std::ofstream ofs;  // save the framebuffer to file
  ofs.open(filepath);

  ofs << "P6\n" << width << " " << height << "\n255\n";
  for (size_t i = 0; i < height * width; ++i) {
    for (size_t j = 0; j < 3; j++) {
      ofs << (char)(255 * std::max(0.f, std::min(1.f, framebuffer[i][j])));
    }
  }
  ofs.close();
}

void saveToPNG(std::vector<Vec3f> framebuffer, int width, int height,
               std::string filepath) {
  unsigned char* data = new unsigned char[width * height * 3];

  for (size_t i = 0; i < height * width; ++i) {
    for (size_t j = 0; j < 3; j++) {
      data[i * 3 + j] =
          (unsigned char)(255 *
                          std::max(0.f, std::min(1.f, framebuffer[i][j])));
    }
  }

  if (!writeToPNG(filepath.c_str(), data, width, height, 3)) {
    std::cerr << "Failed to write PNG to " << filepath.c_str() << std::endl;
  };

  free(data);
}

int64_t nowMillis() {
  auto epoch = std::chrono::system_clock::now().time_since_epoch();
  auto millis =
      std::chrono::duration_cast<std::chrono::milliseconds>(epoch).count();
  return millis;
}
