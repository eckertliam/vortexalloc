cmake_minimum_required(VERSION 3.26)

project(VortexAlloc
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.8.1
)

FetchContent_MakeAvailable(Catch2)

# Enable testing
include(CTest)
enable_testing()
include(Catch)

# Main library (header-only)
add_library(vortexalloc INTERFACE)
target_sources(vortexalloc INTERFACE 
      include/vortexalloc/allocator.hpp
      include/vortexalloc/chunk.hpp)

target_include_directories(vortexalloc INTERFACE include)

# Tests
add_executable(tests tests/arena_tests.cpp)
target_link_libraries(tests PRIVATE vortexalloc)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)
catch_discover_tests(tests)

# Benchmark
add_executable(bench benchmark/arena_bench.cpp)
target_link_libraries(bench PRIVATE vortexalloc)

set_target_properties(vortexalloc PROPERTIES FOLDER "Libs")
set_target_properties(tests PROPERTIES FOLDER "Tests")
set_target_properties(bench PROPERTIES FOLDER "Benchmarks")