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

  void *allocate(std::size_t size,
                 std::size_t align = alignof(std::max_align_t)) {
    std::size_t aligned_offset = (offset + align - 1) & ~(align - 1);
    // if this chunk doesnt have enough space just allocate a new one
    // and return the pointer to the new chunk
    if (aligned_offset + size > capacity) {
      if (!next) {
        // if the size of the object is larger than our standard chunk size
        // allocate a new chunk with the size of the object and its padding
        next = new Chunk(std::max(size, capacity));
      }
      return next->allocate(size, align);
    }

    void *ptr = memory + aligned_offset;
    offset = aligned_offset + size;
    return ptr;
  }
};