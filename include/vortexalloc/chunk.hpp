#pragma once

#include <algorithm>
#include <cstddef>

struct Chunk {
  Chunk *next;
  std::byte *memory;
  std::size_t capacity;
  std::size_t offset;

  Chunk(std::size_t capacity)
      : next(nullptr), memory(new std::byte[capacity]), capacity(capacity),
        offset(0) {}

  ~Chunk() { delete[] memory; }

  void *try_allocate(std::size_t size,
                     std::size_t align = alignof(std::max_align_t)) noexcept {
    std::size_t aligned_offset = (offset + align - 1) & ~(align - 1);
    if (aligned_offset + size > capacity) {
      return nullptr;
    }
    void *ptr = memory + aligned_offset;
    offset = aligned_offset + size;
    return ptr;
  }

  void *allocate(std::size_t size,
                 std::size_t align = alignof(std::max_align_t)) {
    void *ptr = try_allocate(size, align);
    if (!ptr) {
      throw std::bad_alloc();
    }
    return ptr;
  }

  Chunk *alloc_next(std::size_t obj_size) noexcept {
    next = new Chunk(std::max(obj_size, capacity));
    return next;
  }
};