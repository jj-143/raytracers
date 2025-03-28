#pragma once

#include <string>

float gammaTransform(float value, float gamma);

/*
 * Save framebuffer to "./{filename}.ppm"
 */
void saveToPPM(std::vector<Vec3f> framebuffer, int width, int height,
               std::string filepath = "out.ppm");

void saveToPNG(std::vector<Vec3f> framebuffer, int width, int height,
               std::string filepath = "out.png");

int64_t nowMillis();
