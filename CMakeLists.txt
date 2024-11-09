cmake_minimum_required(VERSION 3.6.0)
project(main VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenMP REQUIRED)

file(GLOB_RECURSE src
  "src/*.h"
  "src/*.cpp"
)

add_executable(
  main
  ${src}
)

if(OpenMP_CXX_FOUND)
  target_link_libraries(main PRIVATE OpenMP::OpenMP_CXX)
endif()