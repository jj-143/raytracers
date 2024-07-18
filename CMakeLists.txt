cmake_minimum_required(VERSION 3.6.0)
project(main VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


file(GLOB src
  "./*.h"
  "./*.cpp"
)

add_executable(
  main
  ${src}
)