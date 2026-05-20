#include <optional>

#include "cli.h"
#include "examples.h"
#include "renderer/Renderer.h"

int main(int argc, char* argv[]) {
  std::optional<CliArgs> args = parseArgs(argc, argv);
  if (!args) exit(2);

  printf("example: \"%s\"\n", args->example.c_str());
  printf("frame size: (%d, %d)\n", args->width, args->height);
  printf("spp: %d\n", args->spp);
  printf("seed: %d\n", args->seed);
  printf("\n");

  std::optional<example::Project> project =
      example::makeExampleProject(args.value());
  if (!project) exit(1);

  Renderer renderer(project->tracer);
  renderer.config = project->config;

  printf("Render started.\n");

  renderer.render(*project->scene);

  example::saveRender(renderer, *project);

  renderer.destroy();
}
