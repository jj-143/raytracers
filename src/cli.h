#pragma once

#include <string>

struct CliArgs {
  std::string example;  // see: examples.h
  int spp;
  int width;
  int height;
  int seed;
};

inline CliArgs parseArgs(int argc, char *argv[]) {
  std::string example = argc > 1 ? argv[1] : "";
  int spp = argc > 2 ? atoi(argv[2]) : 10;
  int width = argc > 3 ? atoi(argv[3]) : 512;
  int height = argc > 4 ? atoi(argv[4]) : 512;
  int seed = argc > 5 ? atoi(argv[5]) : 0;

  return {.example = example,
          .spp = spp,
          .width = width,
          .height = height,
          .seed = seed};
}

inline void printUsage() {
  std::string usage =
      "Usage: main <example> [<spp>] [<width>] [<height>] [<seed>]\n"
      "\n"
      "Options:\n"
      "  example\t\"whitted\", \"recursive\", \"simple\" (roughness=20), "
      "\"simple0\" (roughness=0), \"simple10\" (roughness=10)\n"
      "  spp\t\tnumber of samples per pixel (default: 10)\n"
      "  width\t\twidth of the rendered output in pixel (default: 512)\n"
      "  height\theight of the rendered output in pixel (default: 512)\n"
      "  seed\t\tseed for RNG for renderer (default: 0)\n";

  printf("%s", usage.c_str());
}