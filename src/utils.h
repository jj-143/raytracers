#pragma once

#include <cstdint>
#include <string>

void gammaEncode(float* framebuffer, int width, int height, float gamma);

/*
 * Save framebuffer to "./{filename}.ppm"
 */
void saveToPPM(float* framebuffer, int width, int height,
               std::string filepath = "out.ppm");

void saveToPNG(float* framebuffer, int width, int height,
               std::string filepath = "out.png");

int64_t nowMillis();
