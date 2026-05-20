cmake_minimum_required(VERSION 3.24.0)
set(PROJECT_NAME "raytracers")
project(${PROJECT_NAME} VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-O3)

add_executable(${PROJECT_NAME})

find_package(OpenMP REQUIRED)

file(GLOB_RECURSE src "src/*.cpp")

set(inc
  "src/core"
  "src/renderer"
  "src"
  "dependencies"
)

# argparse
include(FetchContent)
FetchContent_Declare(
  argparse
  GIT_REPOSITORY https://github.com/p-ranav/argparse.git
  GIT_TAG v3.2
)
FetchContent_MakeAvailable(argparse)

if (USE_CUDA)
  message("raytracers: USE_CUDA ON")

  enable_language (CUDA)
  set(CMAKE_CUDA_STANDARD 20)

  file(GLOB_RECURSE cuda_src "src/*.cu")
  list(APPEND src ${cuda_src})
  list(APPEND inc "src/cuda")

  set_source_files_properties (
    ${src}
    PROPERTIES LANGUAGE CUDA
  )

  target_compile_definitions(${PROJECT_NAME} PRIVATE GPU_RENDER)

  # Add include paths to nvcc in `compile_commands.json`
  set(CMAKE_CUDA_USE_RESPONSE_FILE_FOR_INCLUDES 0)

  set_target_properties(${PROJECT_NAME} PROPERTIES CUDA_ARCHITECTURES native)
  set_target_properties(${PROJECT_NAME} PROPERTIES CUDA_SEPARABLE_COMPILATION ON)

  target_compile_options(${PROJECT_NAME} PRIVATE $<$<COMPILE_LANG_AND_ID:CUDA,NVIDIA>:--extended-lambda>)

  find_package(CCCL)
  target_link_libraries(${PROJECT_NAME} PRIVATE CCCL::CCCL)
endif()

target_sources(${PROJECT_NAME} PRIVATE ${src})
target_include_directories(${PROJECT_NAME} PRIVATE ${inc})
target_link_libraries(${PROJECT_NAME} PRIVATE OpenMP::OpenMP_CXX argparse)
