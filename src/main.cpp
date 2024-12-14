/**
 * NOTE:
 * Framebuffer origin (0, 0) at top-left.
 * Screen space origin (0, 0) at bottom-left:
 *  - Right (+X), Up (+Y), Facing Out from Screen (+Z)
 */

#include <cmath>
#include <iostream>
#include <limits>
#include <thread>
#include <vector>

#include "core/Material.h"
#include "core/Mesh.h"
#include "math.h"
#include "renderer/Renderer.h"
#include "renderer/Scene.h"
#include "renderer/SceneObject.h"
#include "renderer/Tracer.h"
#include "utils.h"

std::shared_ptr<Scene> makeCornellBoxScene() {
  std::shared_ptr<Scene> scene = std::make_shared<Scene>();

  scene->camera = Camera{.pos = {.275, .275, .8}, .fov = 39.3076 * M_PI / 180};

  scene->lights.push_back(
      std::make_shared<PointLight>(Vec3f(.275, .548, -.275), Vec3f(1)));

  // clang-format off
  auto sRed      = std::make_shared<Sphere>(Vec3f(.425, .080, -.344), .08, "Sphere.red");
  auto sGreen    = std::make_shared<Sphere>(Vec3f(.125, .080, -.344), .08, "Sphere.green");
  auto sYellow   = std::make_shared<Sphere>(Vec3f(.195, .275, -.470), .08, "Sphere.yellow");
  auto sMirror   = std::make_shared<Sphere>(Vec3f(.355, .275, -.470), .08, "Sphere.mirror");
  auto sGlass    = std::make_shared<Sphere>(Vec3f(.275, .080, -.140), .08, "Sphere.glass");
  auto floor     = std::make_shared<Plane> (Vec3f(.275,    0, -.275), .275, .275, Vec3f(0, 1, 0), "Plane.floor");
  auto ceiling   = std::make_shared<Plane> (Vec3f(.275,  .55, -.275), .275, .275, Vec3f(0,-1, 0), "Plane.ceiling");
  auto wallLeft  = std::make_shared<Plane> (Vec3f(   0, .275, -.275), .275, .275, Vec3f(1, 0, 0), "Plane.wallLeft");
  auto wallRight = std::make_shared<Plane> (Vec3f( .55, .275, -.275), .275, .275, Vec3f(-1,0, 0), "Plane.wallRight");
  auto wallBack  = std::make_shared<Plane> (Vec3f(.275, .275, -.550), .275, .275, Vec3f( 0,0, 1), "Plane.wallBack");

  auto red       = std::make_shared<PhongMaterial>(PhongMaterial{Vec3f(.5,  0,  0), {.3, .01,  0},  800,   1});
  auto green     = std::make_shared<PhongMaterial>(PhongMaterial{Vec3f( 0, .5,  0), {.3, .01,  0},  800,   1});
  auto yellow    = std::make_shared<PhongMaterial>(PhongMaterial{Vec3f(.5, .5,  0), {.3, .01,  0},  800,   1});
  auto white     = std::make_shared<PhongMaterial>(PhongMaterial{Vec3f(.5, .5, .5), {.3, .01,  0},  800,   1});
  auto mirror    = std::make_shared<PhongMaterial>(PhongMaterial{Vec3f( 0,  0,  0), {10,  .8,  0}, 1500,   1});
  auto glass     = std::make_shared<PhongMaterial>(PhongMaterial{Vec3f( 0,  0,  0), {10, .01, .8}, 1500, 1.5});
  // clang-format on

  scene->add(std::make_shared<SceneObject>(sRed, red));
  scene->add(std::make_shared<SceneObject>(sGreen, green));
  scene->add(std::make_shared<SceneObject>(sYellow, yellow));
  scene->add(std::make_shared<SceneObject>(sMirror, mirror));
  scene->add(std::make_shared<SceneObject>(sGlass, glass));
  scene->add(std::make_shared<SceneObject>(floor, white));
  scene->add(std::make_shared<SceneObject>(ceiling, white));
  scene->add(std::make_shared<SceneObject>(wallLeft, red));
  scene->add(std::make_shared<SceneObject>(wallRight, green));
  scene->add(std::make_shared<SceneObject>(wallBack, white));

  return scene;
}

int main(int argc, char *argv[]) {
  int width = argc > 1 ? atoi(argv[1]) : 512;
  int height = argc > 2 ? atoi(argv[2]) : 512;
  int spp = argc > 3 ? atoi(argv[3]) : 4;
  int seed = argc > 4 ? atoi(argv[4]) : 0;

  printf("Frame size: (%d, %d)\n", width, height);

  if (!(width > 0 && height > 0)) {
    fprintf(stderr, "Error: invalid argument: [width height]\n");
    fprintf(stderr, "usage: main [width height]\n");
    return 2;
  }

  std::shared_ptr<Scene> scene = makeCornellBoxScene();
  std::shared_ptr<Tracer> tracer = std::make_shared<WhittedRaytracer>();

  Renderer renderer(tracer);
  renderer.config = {width, height, spp, seed};
  renderer.render(*scene);
  save(renderer.framebuffer, width, height);
  return 0;
}
