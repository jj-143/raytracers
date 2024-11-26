#pragma once

#include <format>
#include <memory>
#include <optional>
#include <string>

#include "cli.h"
#include "core/Material.h"
#include "core/Mesh.h"
#include "math.h"
#include "renderer/Renderer.h"
#include "renderer/Scene.h"
#include "renderer/SceneObject.h"
#include "renderer/Tracer.h"
#include "utils.h"

namespace example {
struct Project {
  std::shared_ptr<Scene> scene;
  std::shared_ptr<Tracer> tracer;
  RenderConfig config;
  std::string name;
};

inline void saveRender(const Renderer &renderer, const Project &project) {
  auto &args = project.config;
  auto name = project.name.empty() ? "out" : project.name;
  std::string filename = name;
  filename += std::format("_{:d}spp", args.spp);

  save(renderer.framebuffer, renderer.config.width, renderer.config.height,
       filename);

  printf("Saved to file \"%s.ppm\"\n", filename.c_str());
}

// Cornell Box Scene for WhittedRaytracer with PhongMaterial + PointLight
inline std::shared_ptr<Scene> CornellBoxPhong() {
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

// Cornell Box Scene with BSDF Material + emissive plane as area light
inline std::shared_ptr<Scene> CornellBoxBSDF() {
  std::shared_ptr<Scene> scene = std::make_shared<Scene>();

  scene->camera = Camera{.pos = {.275, .275, .8}, .fov = 39.3076 * M_PI / 180};

  // clang-format off
  auto sRed      = std::make_shared<Sphere>(Vec3f(.425, .080, -.344), .080, "Sphere.red");
  auto sGreen    = std::make_shared<Sphere>(Vec3f(.125, .080, -.344), .080, "Sphere.green");
  auto sYellow   = std::make_shared<Sphere>(Vec3f(.195, .275, -.470), .080, "Sphere.yellow");
  auto sMirror   = std::make_shared<Sphere>(Vec3f(.355, .275, -.470), .080, "Sphere.mirror");
  auto sGlass    = std::make_shared<Sphere>(Vec3f(.275, .080, -.140), .080, "Sphere.glass");
  auto floor     = std::make_shared<Plane> (Vec3f(.275,    0, -.275), .275, .275, Vec3f(0, 1, 0), "Plane.floor");
  auto ceiling   = std::make_shared<Plane> (Vec3f(.275,  .55, -.275), .275, .275, Vec3f(0,-1, 0), "Plane.ceiling");
  auto wallLeft  = std::make_shared<Plane> (Vec3f(   0, .275, -.275), .275, .275, Vec3f(1, 0, 0), "Plane.wallLeft");
  auto wallRight = std::make_shared<Plane> (Vec3f( .55, .275, -.275), .275, .275, Vec3f(-1,0, 0), "Plane.wallRight");
  auto wallBack  = std::make_shared<Plane> (Vec3f(.275, .275, -.550), .275, .275, Vec3f( 0,0, 1), "Plane.wallBack");
  auto areaLight = std::make_shared<Plane> (Vec3f(.275, .548, -.275), .065, .065, Vec3f(0,-1, 0), "Plane.AreaLight");

  auto red       = std::make_shared<LambertBSDF>(Vec3f(.5,  0,  0));
  auto green     = std::make_shared<LambertBSDF>(Vec3f( 0, .5,  0));
  auto yellow    = std::make_shared<LambertBSDF>(Vec3f(.5, .5,  0));
  auto white     = std::make_shared<LambertBSDF>(Vec3f(.5, .5, .5));
  auto light     = std::make_shared<Emission>(Vec3f(25));

  // clang-format on
  scene->add(std::make_shared<SceneObject>(sRed, red));
  scene->add(std::make_shared<SceneObject>(sGreen, green));
  scene->add(std::make_shared<SceneObject>(sYellow, yellow));
  scene->add(std::make_shared<SceneObject>(sMirror, white));
  scene->add(std::make_shared<SceneObject>(sGlass, white));
  scene->add(std::make_shared<SceneObject>(floor, white));
  scene->add(std::make_shared<SceneObject>(ceiling, white));
  scene->add(std::make_shared<SceneObject>(wallLeft, red));
  scene->add(std::make_shared<SceneObject>(wallRight, green));
  scene->add(std::make_shared<SceneObject>(wallBack, white));
  scene->add(std::make_shared<LightObject>(areaLight, light));

  return scene;
}

inline Project ProjectWhittedRaytracer() {
  return {.scene = CornellBoxPhong(),
          .tracer = std::make_shared<WhittedRaytracer>(),
          .name = "whitted_raytracer"};
}

inline Project ProjectRecursivePathtracer() {
  return {.scene = CornellBoxBSDF(),
          .tracer = std::make_shared<RecursivePathtracer>(),
          .name = "recursive_pathtracer"};
}

inline std::optional<Project> makeExampleProject(CliArgs args) {
  std::optional<Project> project =
      args.example == "whitted"     ? ProjectWhittedRaytracer()
      : args.example == "recursive" ? ProjectRecursivePathtracer()
                                    : std::optional<Project>{};
  if (!project) return {};

  project->config = {.width = args.width,
                     .height = args.height,
                     .spp = args.spp,
                     .seed = args.seed};
  return project;
}
}  // namespace example
