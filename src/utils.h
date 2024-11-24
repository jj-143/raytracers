#pragma once

#include <string>

float gammaTransform(float value, float gamma);

/*
 * Save framebuffer to "./{filename}.ppm"
 */
void save(std::vector<Vec3f> framebuffer, int width, int height,
          std::string filename = "out");

int64_t nowMillis();
