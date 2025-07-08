#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include "vortexalloc/allocator.hpp"
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <list>
#include <string>
#include <vector>

constexpr std::size_t N = 1000000;

struct LargeObject {
  std::uint8_t data[512];
};

TEST_CASE("ChunkAllocator Benchmark - push_back ints") {
  BENCHMARK("std::vector<int> with std::allocator") {
    std::vector<int> v;
    v.reserve(N);
    for (std::size_t i = 0; i < N; ++i)
      v.push_back(i);
  };

  BENCHMARK("std::vector<int> with ChunkAllocator") {
    std::vector<int, ChunkAllocator<int>> v;
    v.reserve(N);
    for (std::size_t i = 0; i < N; ++i)
      v.push_back(i);
  };
}

TEST_CASE("ChunkAllocator Benchmark - construct small objects") {
  BENCHMARK("std::list<char> with std::allocator") {
    std::list<char> l;
    for (std::size_t i = 0; i < N; ++i)
      l.push_back(static_cast<char>(i));
  };

  BENCHMARK("std::list<char> with ChunkAllocator") {
    std::list<char, ChunkAllocator<char>> l;
    for (std::size_t i = 0; i < N; ++i)
      l.push_back(static_cast<char>(i));
  };
}

TEST_CASE("ChunkAllocator Benchmark - large object stress") {
  BENCHMARK("std::vector<LargeObject> with std::allocator") {
    std::vector<LargeObject> v;
    v.reserve(N / 100); // Fewer large objects
    for (std::size_t i = 0; i < N / 100; ++i)
      v.emplace_back();
  };

  BENCHMARK("std::vector<LargeObject> with ChunkAllocator") {
    std::vector<LargeObject, ChunkAllocator<LargeObject>> v;
    v.reserve(N / 100);
    for (std::size_t i = 0; i < N / 100; ++i)
      v.emplace_back();
  };
}

TEST_CASE("ChunkAllocator Benchmark - reset reuse") {
  ChunkAllocator<int> alloc(1024 * 1024);

  BENCHMARK("Allocate, reset, and reuse with ChunkAllocator") {
    for (std::size_t round = 0; round < 10; ++round) {
      std::vector<int, ChunkAllocator<int>> v(alloc);
      for (std::size_t i = 0; i < N / 10; ++i)
        v.push_back(i);
      alloc.reset();
    }
  };
}