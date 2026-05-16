#pragma once

#include <format>
#include <memory>
#include <optional>
#include <string>

#include "Material.h"
#include "Mesh.h"
#include "Renderer.h"
#include "Scene.h"
#include "SceneObject.h"
#include "Tracer.h"
#include "cli.h"
#include "math.h"
#include "utils.h"

namespace example {
struct Project {
  std::shared_ptr<Scene> scene;
  std::shared_ptr<Tracer> tracer;
  RenderConfig config;
  std::string name;
};

inline void saveRender(const Renderer& renderer, const Project& project) {
  auto& args = project.config;
  auto name = project.name.empty() ? "out" : project.name;
  std::string filename = std::format("{:s}_{:d}spp", name, args.spp);

  // Save to PNG
  std::string filepath = filename + ".png";
  saveToPNG(renderer.framebuffer, renderer.config.width, renderer.config.height,
            filepath);

  printf("Saved to file \"%s\"\n", filepath.c_str());
}

// Cornell Box Scene for WhittedRaytracer with PhongMaterial + PointLight
inline std::shared_ptr<Scene> CornellBoxPhong() {
  std::shared_ptr<Scene> scene = std::make_shared<Scene>();

  scene->camera = Camera{.pos = {.275, .275, .8}, .fov = 39.3076 * M_PI / 180};

  auto& alloc = scene->allocator;

  // clang-format off
  auto sRed      = alloc.emplaceMesh<Sphere>(Vec3f(.425, .080, -.344), .08);
  auto sGreen    = alloc.emplaceMesh<Sphere>(Vec3f(.125, .080, -.344), .08);
  auto sYellow   = alloc.emplaceMesh<Sphere>(Vec3f(.195, .275, -.470), .08);
  auto sMirror   = alloc.emplaceMesh<Sphere>(Vec3f(.355, .275, -.470), .08);
  auto sGlass    = alloc.emplaceMesh<Sphere>(Vec3f(.275, .080, -.140), .08);
  auto floor     = alloc.emplaceMesh<Plane> (Vec3f(.275,    0, -.275), .275, .275, Vec3f(0, 1, 0));
  auto ceiling   = alloc.emplaceMesh<Plane> (Vec3f(.275,  .55, -.275), .275, .275, Vec3f(0,-1, 0));
  auto wallLeft  = alloc.emplaceMesh<Plane> (Vec3f(   0, .275, -.275), .275, .275, Vec3f(1, 0, 0));
  auto wallRight = alloc.emplaceMesh<Plane> (Vec3f( .55, .275, -.275), .275, .275, Vec3f(-1,0, 0));
  auto wallBack  = alloc.emplaceMesh<Plane> (Vec3f(.275, .275, -.550), .275, .275, Vec3f( 0,0, 1));

  auto red       = alloc.emplaceMaterial<PhongMaterial>(PhongMaterial{Vec3f(.5,  0,  0), {.3, .01,  0},  800,   1});
  auto green     = alloc.emplaceMaterial<PhongMaterial>(PhongMaterial{Vec3f( 0, .5,  0), {.3, .01,  0},  800,   1});
  auto yellow    = alloc.emplaceMaterial<PhongMaterial>(PhongMaterial{Vec3f(.5, .5,  0), {.3, .01,  0},  800,   1});
  auto white     = alloc.emplaceMaterial<PhongMaterial>(PhongMaterial{Vec3f(.5, .5, .5), {.3, .01,  0},  800,   1});
  auto mirror    = alloc.emplaceMaterial<PhongMaterial>(PhongMaterial{Vec3f( 0,  0,  0), {10,  .8,  0}, 1500,   1});
  auto glass     = alloc.emplaceMaterial<PhongMaterial>(PhongMaterial{Vec3f( 0,  0,  0), {10, .01, .8}, 1500, 1.5});
  // clang-format on

  alloc.emplaceObject<BaseObject>(sRed, red);
  alloc.emplaceObject<BaseObject>(sGreen, green);
  alloc.emplaceObject<BaseObject>(sYellow, yellow);
  alloc.emplaceObject<BaseObject>(sMirror, mirror);
  alloc.emplaceObject<BaseObject>(sGlass, glass);
  alloc.emplaceObject<BaseObject>(floor, white);
  alloc.emplaceObject<BaseObject>(ceiling, white);
  alloc.emplaceObject<BaseObject>(wallLeft, red);
  alloc.emplaceObject<BaseObject>(wallRight, green);
  alloc.emplaceObject<BaseObject>(wallBack, white);

  auto emission = alloc.emplaceMaterial<Emission>(Vec3f(1));
  auto pointLight = alloc.emplaceMesh<Sphere>(Vec3f(.275, .548, -.275), 0);
  alloc.emplaceObject<Light>(pointLight, emission);

  return scene;
}

// Cornell Box Scene with BSDF Material + emissive plane as area light
inline std::shared_ptr<Scene> CornellBoxBSDF(float roughness = .2) {
  std::shared_ptr<Scene> scene = std::make_shared<Scene>();

  scene->camera = Camera{.pos = {.275, .275, .8}, .fov = 39.3076 * M_PI / 180};

  auto& alloc = scene->allocator;

  // clang-format off
  auto sRed      = alloc.emplaceMesh<Sphere>(Vec3f(.425, .080, -.344), .080);
  auto sGreen    = alloc.emplaceMesh<Sphere>(Vec3f(.125, .080, -.344), .080);
  auto sYellow   = alloc.emplaceMesh<Sphere>(Vec3f(.195, .275, -.470), .080);
  auto sMirror   = alloc.emplaceMesh<Sphere>(Vec3f(.355, .275, -.470), .080);
  auto sGlass    = alloc.emplaceMesh<Sphere>(Vec3f(.275, .080, -.140), .080);
  auto floor     = alloc.emplaceMesh<Plane> (Vec3f(.275,    0, -.275), .275, .275, Vec3f(0, 1, 0));
  auto ceiling   = alloc.emplaceMesh<Plane> (Vec3f(.275,  .55, -.275), .275, .275, Vec3f(0,-1, 0));
  auto wallLeft  = alloc.emplaceMesh<Plane> (Vec3f(   0, .275, -.275), .275, .275, Vec3f(1, 0, 0));
  auto wallRight = alloc.emplaceMesh<Plane> (Vec3f( .55, .275, -.275), .275, .275, Vec3f(-1,0, 0));
  auto wallBack  = alloc.emplaceMesh<Plane> (Vec3f(.275, .275, -.550), .275, .275, Vec3f( 0,0, 1));
  auto areaLight = alloc.emplaceMesh<Plane> (Vec3f(.275, .548, -.275), .065, .065, Vec3f(0,-1, 0));

  auto red       = alloc.emplaceMaterial<GlossyDiffuseLambertBSDF>(Vec3f(.5,  0,  0), roughness, 1.5);
  auto green     = alloc.emplaceMaterial<GlossyDiffuseLambertBSDF>(Vec3f( 0, .5,  0), roughness, 1.5);
  auto yellow    = alloc.emplaceMaterial<GlossyDiffuseLambertBSDF>(Vec3f(.5, .5,  0), roughness, 1.5);
  auto white     = alloc.emplaceMaterial<GlossyDiffuseLambertBSDF>(Vec3f(.5, .5, .5), roughness, 1.5);
  auto mirror    = alloc.emplaceMaterial<MetalBSDF>(Vec3f(1, 1, 1));
  auto glass     = alloc.emplaceMaterial<DielectricBSDF>(1.5);
  auto light     = alloc.emplaceMaterial<Emission>(Vec3f(25));

  // clang-format on
  alloc.emplaceObject<BaseObject>(sRed, red);
  alloc.emplaceObject<BaseObject>(sGreen, green);
  alloc.emplaceObject<BaseObject>(sYellow, yellow);
  alloc.emplaceObject<BaseObject>(sMirror, mirror);
  alloc.emplaceObject<BaseObject>(sGlass, glass);
  alloc.emplaceObject<BaseObject>(floor, white);
  alloc.emplaceObject<BaseObject>(ceiling, white);
  alloc.emplaceObject<BaseObject>(wallLeft, red);
  alloc.emplaceObject<BaseObject>(wallRight, green);
  alloc.emplaceObject<BaseObject>(wallBack, white);
  alloc.emplaceObject<Light>(areaLight, light);

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

inline Project ProjectSimplePathtracer(float roughness) {
  return {.scene = CornellBoxBSDF(roughness),
          .tracer = std::make_shared<SimplePathtracer>(),
          .name = std::format("simple_pathtracer_r{:d}", int(roughness * 100))};
}

inline std::optional<Project> makeExampleProject(CliArgs args) {
  std::optional<Project> project =
      args.example == "whitted"     ? ProjectWhittedRaytracer()
      : args.example == "recursive" ? ProjectRecursivePathtracer()
      : args.example == "simple"    ? ProjectSimplePathtracer(.2)
      : args.example == "simple10"  ? ProjectSimplePathtracer(.1)
      : args.example == "simple0"   ? ProjectSimplePathtracer(0)
                                    : std::optional<Project>{};
  if (!project) return {};

  project->config = {.width = args.width,
                     .height = args.height,
                     .spp = args.spp,
                     .seed = args.seed};
  return project;
}
}  // namespace example
