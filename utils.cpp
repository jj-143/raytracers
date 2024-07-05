#include <fstream>
#include <iostream>
#include <vector>

#include "geometry.h"

/*
* Save framebuffer to file "build/out.ppm"
*/
void save(std::vector<Vec3f> framebuffer, int width, int height) {
  std::ofstream ofs;  // save the framebuffer to file
  ofs.open("./out.ppm");
  ofs << "P6\n" << width << " " << height << "\n255\n";
  for (size_t i = 0; i < height * width; ++i) {
    for (size_t j = 0; j < 3; j++) {
      ofs << (char)(255 * std::max(0.f, std::min(1.f, framebuffer[i][j])));
    }
  }
  ofs.close();
}