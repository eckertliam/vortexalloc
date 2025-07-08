#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>

#include "vortexalloc/allocator.hpp"

#include <cstdint>
#include <vector>

// Helper struct to track construction and destruction
struct Tracer {
  static int ctor_count;
  static int dtor_count;
  int value;
  explicit Tracer(int v) : value(v) { ++ctor_count; }
  ~Tracer() { ++dtor_count; }
};
int Tracer::ctor_count = 0;
int Tracer::dtor_count = 0;

TEST_CASE("Zero‑allocation returns non‑null", "[ChunkAllocator]") {
  ChunkAllocator<int> alloc;
  auto *p = alloc.allocate(0);
  REQUIRE(p != nullptr);
}

TEST_CASE("Basic allocation/construct/destroy of POD", "[ChunkAllocator]") {
  ChunkAllocator<int> alloc;
  int *p = alloc.allocate(1);
  REQUIRE(p != nullptr);
  REQUIRE(reinterpret_cast<std::uintptr_t>(p) % alignof(int) == 0);

  alloc.construct(p, 42);
  REQUIRE(*p == 42);
  alloc.destroy(p);
}

TEST_CASE("Alignment is respected for over‑aligned types", "[ChunkAllocator]") {
  struct alignas(64) Aligned64 {
    std::uint64_t data[8]{};
  };

  ChunkAllocator<Aligned64> alloc;
  Aligned64 *p = alloc.allocate(1);
  REQUIRE(p != nullptr);
  REQUIRE(reinterpret_cast<std::uintptr_t>(p) % alignof(Aligned64) == 0);
}

TEST_CASE("max_size + 1 throws std::bad_alloc", "[ChunkAllocator]") {
  ChunkAllocator<int> alloc;
  REQUIRE_THROWS_AS(alloc.allocate(alloc.max_size() + 1), std::bad_alloc);
}

TEST_CASE("Allocator works with std::vector", "[ChunkAllocator][STL]") {
  ChunkAllocator<int> alloc;
  std::vector<int, ChunkAllocator<int>> vec{alloc};

  const int N = 10'000;
  for (int i = 0; i < N; ++i) {
    vec.push_back(i);
  }

  REQUIRE(vec.size() == static_cast<std::size_t>(N));
  for (int i = 0; i < N; ++i) {
    REQUIRE(vec[i] == i);
  }
}

TEST_CASE("Object lifetime accounting", "[ChunkAllocator]") {
  Tracer::ctor_count = 0;
  Tracer::dtor_count = 0;

  constexpr int N = 128;
  {
    ChunkAllocator<Tracer> alloc;
    Tracer *p = alloc.allocate(N);
    for (int i = 0; i < N; ++i) {
      alloc.construct(p + i, i);
    }
    REQUIRE(Tracer::ctor_count == N);

    for (int i = 0; i < N; ++i) {
      alloc.destroy(p + i);
    }
    REQUIRE(Tracer::dtor_count == N);
  }

  REQUIRE(Tracer::ctor_count == Tracer::dtor_count);
}

TEST_CASE("reset reuses the current chunk", "[ChunkAllocator]") {
  ChunkAllocator<int> alloc(1024); // small chunk for the test
  int *first = alloc.allocate(16);
  alloc.reset();
  int *second = alloc.allocate(16);

  // After reset the allocator should hand out the same address again
  REQUIRE(first == second);
}