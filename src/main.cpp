#include <optional>

#include "cli.h"
#include "examples.h"
#include "renderer/Renderer.h"

#ifdef GPU_RENDER
#include "cuda/Renderer.h"
#endif

void renderWithCPU(example::Project& project) {
  Renderer renderer(project.config);

  renderer.render(*project.scene);
  example::saveRender(renderer.framebuffer, project);

  renderer.destroy();
}

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

  printf("Render started.\n");

#ifdef GPU_RENDER
  renderWithGPU(*project);
#else
  renderWithCPU(*project);
#endif
}
