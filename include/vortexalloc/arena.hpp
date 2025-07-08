#pragma once

#include "chunk.hpp"
#include <cstddef>
#include <new>

struct Arena {
  std::size_t chunk_size_ = 1024 * 1024; // default: 1MB
  Chunk *head_;
  Chunk *current_;

  Arena(std::size_t chunk_size)
      : chunk_size_(chunk_size), head_(nullptr), current_(nullptr) {}

  Arena() : head_(nullptr), current_(nullptr) {}

  ~Arena() {
    Chunk *cur = head_;
    while (cur) {
      Chunk *next = cur->next;
      delete cur;
      cur = next;
    }
  }

  void *allocate(std::size_t bytes, std::size_t align) {
    // if current is null allocate a new chunk
    // set head and current to the new chunk
    if (!current_) {
      std::size_t size = std::max(bytes, chunk_size_);
      current_ = new Chunk(size);
      head_ = current_;
    }

    // try toallocate from the current chunk
    void *ptr = current_->try_allocate(bytes, align);

    // if the allocation failed check if the current chunk has a next chunk
    // if so, set current to the next chunk
    if (!ptr && current_->next) {
      current_ = current_->next;
      ptr = current_->try_allocate(bytes, align);
    } else if (!ptr && !current_->next) {
      // if the allocation failed and the current chunk has no next chunk
      // allocate a new chunk
      current_ = current_->alloc_next(bytes);
      ptr = current_->try_allocate(bytes, align);
    }

    if (!ptr) {
      throw std::bad_alloc();
    }

    return ptr;
  }

  void reset() noexcept {
    for (auto *c = head_; c; c = c->next)
      c->offset = 0;
    current_ = head_;
  }
};