#pragma once

#include <optional>
#include <string>

struct CliArgs {
  std::string example;  // see: examples.h
  int spp;
  int width;
  int height;
  int seed;
};

std::optional<CliArgs> parseArgs(int argc, char *argv[]);
