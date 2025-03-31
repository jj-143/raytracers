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

target_include_directories(${PROJECT_NAME}
  PRIVATE dependencies
)

if(OpenMP_CXX_FOUND)
  target_link_libraries(${PROJECT_NAME} PRIVATE OpenMP::OpenMP_CXX)
endif()