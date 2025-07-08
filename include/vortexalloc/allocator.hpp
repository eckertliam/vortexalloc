#pragma once

#include "chunk.hpp"

template <typename T> class ChunkAllocator {
private:
  std::size_t chunk_size_ = 1024 * 1024; // default: 1MB
  Chunk *head_;
  Chunk *current_;

public:
  using value_type = T;
  using pointer = T *;
  using const_pointer = const T *;
  using void_pointer = void *;
  using const_void_pointer = const void *;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

  template <class U> struct rebind {
    using other = ChunkAllocator<U>;
  };

  using propagate_on_container_move_assignment = std::true_type;
  using is_always_equal = std::true_type;

  ChunkAllocator(std::size_t chunk_size)
      : chunk_size_(chunk_size), head_(nullptr), current_(nullptr) {}
  ChunkAllocator() : head_(nullptr), current_(nullptr) {}

  ~ChunkAllocator() {
    Chunk *cur = head_;
    while (cur) {
      Chunk *next = cur->next;
      delete cur;
      cur = next;
    }
  }

  pointer allocate(std::size_t n) {
    if (n > max_size()) {
      throw std::bad_alloc();
    }

    std::size_t bytes = n * sizeof(T);
    std::size_t align = alignof(T);

    // if current is null, allocate a new chunk
    // set head and current to the new chunk
    if (!current_) {
      current_ = new Chunk(chunk_size_);
      head_ = current_;
    }

    // allocate from the current chunk
    void *ptr = current_->allocate(bytes, align);

    // check if the current chunk allocated a new chunk
    // if so, set current to the new chunk
    if (current_->next) {
      current_ = current_->next;
    }

    return static_cast<T *>(ptr);
  }

  // chunk allocator does nothing
  void deallocate(pointer p, std::size_t n) noexcept {}

  size_type max_size() const noexcept {
    return std::numeric_limits<size_type>::max() / sizeof(T);
  }

  void reset() noexcept {
    for (auto *c = head_; c; c = c->next)
      c->offset = 0;
    current_ = head_;
  }
};