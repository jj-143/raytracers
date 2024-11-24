#pragma once

float gammaTransform(float value, float gamma);

void save(std::vector<Vec3f> framebuffer, int width, int height);
int64_t nowMillis();
