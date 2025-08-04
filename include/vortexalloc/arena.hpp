#pragma once

#include "chunk.hpp"
#include <new>

struct Arena {
  std::size_t initial_chunk_size_ = 8 * 1024; // Start with 8KB
  std::size_t max_chunk_size_ = 1024 * 1024;  // Max 1MB
  Chunk *head_;
  Chunk *current_;

  explicit Arena(const std::size_t initial_chunk_size)
      : initial_chunk_size_(initial_chunk_size), head_(nullptr), current_(nullptr) {}

  Arena() : head_(nullptr), current_(nullptr) {}

  ~Arena() {
    const Chunk *cur = head_;
    while (cur) {
      const Chunk *next = cur->next;
      delete cur;
      cur = next;
    }
  }

  void *allocate(const std::size_t bytes, const std::size_t align) {
    // if current is null allocate a new chunk
    // set head and current to the new chunk
    if (!current_) {
      const std::size_t size = std::max(bytes, initial_chunk_size_);
      current_ = new Chunk(size);
      head_ = current_;
    }

    // try to allocate from the current chunk
    void *ptr = current_->try_allocate(bytes, align);

    // if the allocation failed, try to find space in existing chunks first
    if (!ptr) {
      // search through existing chunks for space
      for (Chunk *chunk = head_; chunk; chunk = chunk->next) {
        ptr = chunk->try_allocate(bytes, align);
        if (ptr) {
          current_ = chunk; // Update current to the chunk we found space in
          break;
        }
      }
      
      // if no space found in existing chunks allocate a new one with progressive sizing
      if (!ptr) {
        const std::size_t current_chunk_size = current_->capacity;
        const std::size_t next_chunk_size = std::min(current_chunk_size * 2, max_chunk_size_);
        const std::size_t required_size = std::max(bytes, next_chunk_size);
        current_ = current_->alloc_next(required_size);
        ptr = current_->try_allocate(bytes, align);
      }
    }

    // if the allocation can't be made
    // throw bad_alloc
    if (!ptr) {
      throw std::bad_alloc();
    }

    return ptr;
  }

  void reset() noexcept {
    // iter over chunks and set each offset to 0
    for (auto *c = head_; c; c = c->next)
      c->offset = 0;
    current_ = head_;
  }
};