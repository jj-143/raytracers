/**
 * NOTE:
 * Framebuffer origin (0, 0) at top-left.
 * Screen space origin (0, 0) at bottom-left:
 *  - Right (+X), Up (+Y), Facing Out from Screen (+Z)
 */

#include <cmath>
#include <iostream>
#include <limits>
#include <vector>

#include "common.h"
#include "geometry.h"
#include "utils.h"

/**
 * Return a random float in [0, 1]
 */
inline float randf() {
  return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

const Vec3f BACKGROUND_COLOR = Vec3f(0, 0, 0);
const int MAX_REFLECTION_DEPTH = 4;
const int MAX_REFRACTION_DEPTH = 12;
const int N_SAMPLES = 4;

const float REFRACTIVE_INDEX_ENVIRONMENT = 1;  // air: 1, water: 1.33

bool scene_intersect(const Vec3f &origin, const Vec3f &dir,
                     const std::vector<Object> &objects, Vec3f &hit, Vec3f &N,
                     Object &object) {
  float t0 = MAXFLOAT;
  for (size_t i = 0; i < objects.size(); i++) {
    Mesh *mesh = objects[i].target;
    float temp_dist = 0;
    Vec3f temp_N;
    if (mesh->ray_intersect(origin, dir, temp_dist, temp_N) && temp_dist < t0) {
      t0 = temp_dist;
      N = temp_N;
      hit = dir * t0 + origin;
      object = objects[i];
    }
  }
  return t0 < MAXFLOAT;
}

bool refract(Vec3f d, Vec3f N, float n_i, float n_t, Vec3f &dir) {
  float n = n_i / n_t;
  float cos_i = -d * N;
  float sin_i = sqrtf(1 - cos_i * cos_i);
  if (sin_i > 1 / n) return false;  // total reflection

  float sin_t = n * sin_i;
  float cos_t = sqrtf(1 - sin_t * sin_t);

  Vec3f d_h = (d + N * cos_i).normalize();
  Vec3f d_n = -N;

  dir = d_h * sin_t + d_n * cos_t;
  return true;
}

bool cast_ray(const Vec3f &orig, const Vec3f dir, Vec3f &color,
              const std::vector<Object> &objects,
              const std::vector<PointLight> &lights, int depth_reflection = 0,
              int depth_refraction = 0) {
  Vec3f hit = Vec3f(0, 0, 0);
  Vec3f n = Vec3f(0, 0, 0);
  Object object;

  // Also discard the ray if it exceeds maximum depth as if it doesn't hit
  // anything.
  // Alternative option is to not casting the [reflection | refraction] ray
  // while returning back only the diffuse and specular.
  if (depth_reflection > MAX_REFLECTION_DEPTH ||
      depth_refraction > MAX_REFRACTION_DEPTH ||
      !scene_intersect(orig, dir, objects, hit, n, object)) {
    return false;
  };

  float diff_intensity = 0;
  float spec_intensity = 0;
  Vec3f r = (dir + n * ((dir * n) * -2));

  // reflection
  Vec3f reflection_color = Vec3f(0, 0, 0);

  Vec3f reflection_dir = r.normalize();
  Vec3f reflection_origin = hit + r * 1e-3;

  if (!cast_ray(reflection_origin, reflection_dir, reflection_color, objects,
                lights, depth_reflection + 1, depth_refraction)) {
    reflection_color = BACKGROUND_COLOR;
  };

  // Refraction
  Vec3f refraction_color = Vec3f(0, 0, 0);
  if (object.material.albedo[3] > 0) {
    bool is_inside = (dir * n) > 0;
    float index_i = is_inside ? object.material.refractive_index
                              : REFRACTIVE_INDEX_ENVIRONMENT;
    float index_t = is_inside ? REFRACTIVE_INDEX_ENVIRONMENT
                              : object.material.refractive_index;

    Vec3f dir_refraction;
    if (refract(dir, is_inside ? -n : n, index_i, index_t, dir_refraction)) {
      dir_refraction = dir_refraction.normalize();
      if (!cast_ray(hit + dir_refraction * 1e-3, dir_refraction,
                    refraction_color, objects, lights, depth_reflection,
                    depth_refraction + 1)) {
        refraction_color = BACKGROUND_COLOR;
      }
    }
  }

  for (size_t light_i = 0; light_i < lights.size(); light_i++) {
    const PointLight light = lights[light_i];
    Vec3f dir_light = (light.pos - hit).normalize();

    // shadow
    Vec3f temp_hit = Vec3f(0, 0, 0);
    Vec3f temp_n = Vec3f(0, 0, 0);
    Object temp_object;
    Vec3f shadow_origin = dir_light * n < 0 ? hit - n * 1e-3 : hit + n * 1e-3;
    if (scene_intersect(shadow_origin, dir_light, objects, temp_hit, temp_n,
                        temp_object)) {
      float dist = (temp_hit - shadow_origin).norm();
      if (dist < (light.pos - hit).norm()) {
        continue;
      }
    }

    diff_intensity += std::max(0.f, n * dir_light) * light.intensity;

    spec_intensity +=
        powf(std::max(0.f, r * dir_light), object.material.specular_exponent) *
        light.intensity;
  }

  color = object.material.diffuse_color * diff_intensity *
              object.material.albedo[0] +
          Vec3f(1, 1, 1) * spec_intensity * object.material.albedo[1] +
          reflection_color * object.material.albedo[2] +
          refraction_color * object.material.albedo[3];
  return true;
}

void render(const int &width, const int &height) {
  // Camera
  Vec3f cam_orig = {.275, .275, .8};

  // 35mm lens with 25mm sensor
  float fov = 39.3076 * M_PI / 180;
  float pixel_size = tan(fov / 2) / width;

  std::vector<Vec3f> framebuffer(width * height);
  std::vector<float> depthbuffer(width * height);

  std::vector<PointLight> lights;
  lights.push_back({Vec3f(.275, .548, -.275), 1.5});

  std::vector<Object> objects;

  // clang-format off
  Sphere s1        = {Vec3f(.425, .080, -.344), .08}; // red
  Sphere s2        = {Vec3f(.125, .080, -.344), .08}; // green
  Sphere s3        = {Vec3f(.195, .275, -.470), .08}; // yellow
  Sphere s4        = {Vec3f(.355, .275, -.470), .08}; // mirror
  Sphere s5        = {Vec3f(.275, .080, -.140), .08}; // glass
  Plane floor      = {Vec3f(.275,    0, -.275), .275, .275, Vec3f(0, 1, 0)};
  Plane ceiling    = {Vec3f(.275,  .55, -.275), .275, .275, Vec3f(0, -1, 0)};
  Plane left_wall  = {Vec3f(   0, .275, -.275), .275, .275, Vec3f(1, 0, 0)};
  Plane right_wall = {Vec3f( .55, .275, -.275), .275, .275, Vec3f(-1, 0, 0)};
  Plane back_wall  = {Vec3f(.275, .275,  -.55), .275, .275, Vec3f(0, 0, 1)};

  Material red        = Material{Vec3f(  1,  0,  0), {  .6,  .3,  .01,   0},  100,    1};
  Material green      = Material{Vec3f(  0,  1,  0), {  .6,  .3,  .01,   0},   20,    1};
  Material yellow     = Material{Vec3f(  1,  1,  0), {  .6,  .3,  .01,   0},  100,    1};
  Material white      = Material{Vec3f(  1,  1,  1), {  .6,  .1,  .01,   0},   10,    1};
  Material mirror     = Material{Vec3f(  1,  1,  1), {   0,  10,  .8,   0}, 1500,    1};
  Material glass      = Material{Vec3f( .6, .7, .8), {  .1,  10,  .01,  .8}, 1500,  1.5};
  // clang-format on

  objects.push_back({s1, red});
  objects.push_back({s2, green});
  objects.push_back({s3, yellow});
  objects.push_back({s4, mirror});
  objects.push_back({s5, glass});
  objects.push_back({floor, white});
  objects.push_back({ceiling, white});
  objects.push_back({left_wall, red});
  objects.push_back({right_wall, green});
  objects.push_back({back_wall, white});

  int64_t now = nowMillis();

  // Render Pass
#pragma omp parallel for collapse(2)
  for (size_t j = 0; j < height; j++) {
    for (size_t i = 0; i < width; i++) {
      Vec3f color = Vec3f();

      for (size_t sample_i = 0; sample_i < N_SAMPLES; sample_i++) {
        Vec3f sample_color = Vec3f();
        float x = (2 * (i + 0.5) / (float)width - 1) * tan(fov / 2.);
        float y = -(2 * (j + 0.5) / (float)height - 1) * tan(fov / 2.) *
                  height / (float)width;

        // Random sample inside the pixel;
        x += (randf() - .5) * pixel_size;
        y += (randf() - .5) * pixel_size;

        Vec3f dir = Vec3f(x, y, -1).normalize();

        if (!cast_ray(cam_orig, dir, sample_color, objects, lights)) {
          sample_color = BACKGROUND_COLOR;
        }
        color = color + sample_color;
      }
      color = color * (1.f / N_SAMPLES);
      framebuffer[i + j * width] = color;
    }
  }

  std::cout << "Render took " << (nowMillis() - now) << "ms" << std::endl;

  save(framebuffer, width, height);
}

int main(int argc, char *argv[]) {
  const int width = argc > 1 ? atoi(argv[1]) : 512;
  const int height = argc > 2 ? atoi(argv[2]) : 512;

  printf("Frame size: (%d, %d)\n", width, height);

  if (!(width > 0 && height > 0)) {
    fprintf(stderr, "Error: invalid argument: [width height]\n");
    fprintf(stderr, "usage: main [width height]\n");
    return 2;
  }

  render(width, height);
  return 0;
}