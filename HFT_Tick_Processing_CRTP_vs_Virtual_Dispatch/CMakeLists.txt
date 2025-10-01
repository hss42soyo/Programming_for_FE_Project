cmake_minimum_required(VERSION 3.16)
project(hft_crtp_assignment LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Reasonable "fast" flags; you may also test -Ofast and LTO separately
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  add_compile_options(-O3 -DNDEBUG -march=native -fno-exceptions -fno-rtti)
  # Remove -fno-rtti if you want RTTI elsewhere; virtual calls don’t require RTTI.
endif()

add_executable(hft
  src/main.cpp
)
target_include_directories(hft PRIVATE include)
