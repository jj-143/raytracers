cmake_minimum_required(VERSION 3.13.0)
set(PROJECT_NAME "raytracers")
project(${PROJECT_NAME} VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-O3)

find_package(OpenMP REQUIRED)

file(GLOB_RECURSE src
  "src/*.h"
  "src/*.cpp"
)

add_executable(
  ${PROJECT_NAME}
  ${src}
)

set(inc
  "src/core"
  "src/renderer"
  "src"
  "dependencies"
)

target_include_directories(${PROJECT_NAME} PRIVATE ${inc})

# argparse
include(FetchContent)
FetchContent_Declare(
  argparse
  GIT_REPOSITORY https://github.com/p-ranav/argparse.git
  GIT_TAG v3.2
)
FetchContent_MakeAvailable(argparse)

target_link_libraries(${PROJECT_NAME} PRIVATE OpenMP::OpenMP_CXX argparse)
