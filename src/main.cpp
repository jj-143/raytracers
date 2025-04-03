#include <optional>

#include "cli.h"
#include "examples.h"
#include "renderer/Renderer.h"

int main(int argc, char *argv[]) {
  CliArgs args = parseArgs(argc, argv);

  std::optional<example::Project> project = example::makeExampleProject(args);

  if (!project) {
    printUsage();
    exit(2);
  }

  printf("Frame size: (%d, %d)\n", args.width, args.height);

  Renderer renderer(project->tracer);
  renderer.config = project->config;
  renderer.render(*project->scene);

  example::saveRender(renderer, *project);
}
