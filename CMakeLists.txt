cmake_minimum_required(VERSION 3.20)

project(TDSGRasterLab VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(TDSG_OPENMP "Parallelize rasterization with OpenMP when available" ON)

add_library(tdsg STATIC
  src/image/image.cpp
  src/image/framebuffer.cpp
  src/core/rasterizer.cpp
  src/core/scene.cpp
  src/io/obj_loader.cpp
)

target_include_directories(tdsg PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_compile_features(tdsg PUBLIC cxx_std_20)

if(MSVC)
  target_compile_options(tdsg PRIVATE /W4 /permissive-)
else()
  target_compile_options(tdsg PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(TDSG_OPENMP)
  find_package(OpenMP COMPONENTS CXX QUIET)
  if(OpenMP_CXX_FOUND)
    target_link_libraries(tdsg PUBLIC OpenMP::OpenMP_CXX)
    target_compile_definitions(tdsg PUBLIC TDSG_OPENMP)
    message(STATUS "TDSG: OpenMP enabled")
  else()
    message(STATUS "TDSG: OpenMP not found, running single threaded")
  endif()
endif()

add_executable(tdsg_raster_engine apps/main.cpp)
target_link_libraries(tdsg_raster_engine PRIVATE tdsg)
