#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "utils.h"

#include <stb_write.h>

#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>

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

void gammaEncode(float* fb, int width, int height, float gamma) {
  for (size_t i = 0; i < width; ++i) {
    for (size_t j = 0; j < height; ++j) {
      for (size_t ch = 0; ch < 3; ++ch) {
        int idx = (j * width + i) * 3 + ch;

        if (fb[idx] != fb[idx]) {
          fb[idx] = 0;  // NaN fix
        }
        fb[idx] = gammaTransform(fb[idx], gamma);
      }
    }
  }
}

void saveToPPM(float* framebuffer, int width, int height,
               std::string filepath) {
  std::ofstream ofs;  // save the framebuffer to file
  ofs.open(filepath);

  ofs << "P6\n" << width << " " << height << "\n255\n";
  for (size_t i = 0; i < height * width; ++i) {
    for (size_t j = 0; j < 3; j++) {
      int idx = i * 3 + j;
      ofs << (char)(255 * std::max(0.f, std::min(1.f, framebuffer[idx])));
    }
  }
  ofs.close();
}

void saveToPNG(float* framebuffer, int width, int height,
               std::string filepath) {
  unsigned char* data = new unsigned char[width * height * 3];

  for (size_t i = 0; i < height * width; ++i) {
    for (size_t j = 0; j < 3; j++) {
      int idx = i * 3 + j;
      data[idx] =
          (unsigned char)(255 * std::max(0.f, std::min(1.f, framebuffer[idx])));
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
