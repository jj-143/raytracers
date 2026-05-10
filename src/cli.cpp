#include "cli.h"

#include <argparse/argparse.hpp>

std::optional<CliArgs> parseArgs(int argc, char* argv[]) {
  CliArgs args;
  argparse::ArgumentParser program("raytracers", "1.0",
                                   argparse::default_arguments::help);
  program.at("help").hidden();
  program.set_usage_max_line_width(80);

  /* Args */
  program.add_argument("example")
      .default_value("simple")
      .choices("whitted", "recursive", "simple", "simple0", "simple10")
      .help(
          "\"whitted\", \"recursive\", \"simple\" (roughness=0.2), \n"
          "\"simple0\" (roughness=0), \"simple10\" (roughness=0.1)")
      .nargs(1)
      .store_into(args.example);

  program.add_argument("--spp")
      .default_value<int>(10)
      .help("number of samples per pixel")
      .nargs(1)
      .metavar("<spp>")
      .store_into(args.spp);

  program.add_argument("--width")
      .default_value<int>(512)
      .help("width of rendered image")
      .nargs(1)
      .metavar("<width>")
      .store_into(args.width);

  program.add_argument("--height")
      .default_value<int>(512)
      .help("height of rendered image")
      .nargs(1)
      .metavar("<height>")
      .store_into(args.height);

  program.add_argument("--seed")
      .default_value<int>(0)
      .help("seed for RNG")
      .nargs(1)
      .metavar("<seed>")
      .store_into(args.seed);

  /* Parse */
  try {
    program.parse_args(argc, argv);
  } catch (const std::exception& err) {
    // Prints usage and help
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    return {};
  }

  return args;
}
