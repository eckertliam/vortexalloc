#pragma once

#include "chunk.hpp"

namespace detail {
// One byte of storage, maximally aligned.
inline std::aligned_storage_t<1, alignof(std::max_align_t)> dummy;

inline void *non_null_one_byte() noexcept { return &dummy; }
} // namespace detail

// TODO: make rebinds share the same internal state

template <typename T> class ChunkAllocator {
  template <typename U> friend class ChunkAllocator;

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
  using propagate_on_container_copy_assignment = std::true_type;
  using propagate_on_container_swap = std::true_type;

  template <class U> struct rebind {
    using other = ChunkAllocator<U>;
  };

  using propagate_on_container_move_assignment = std::true_type;
  using is_always_equal = std::true_type;

  ChunkAllocator(std::size_t chunk_size)
      : chunk_size_(chunk_size), head_(nullptr), current_(nullptr) {}
  ChunkAllocator() : head_(nullptr), current_(nullptr) {}

  // Copy constructor – makes a fresh allocator that shares no state
  ChunkAllocator(const ChunkAllocator &other) noexcept
      : chunk_size_(other.chunk_size_), head_(nullptr), current_(nullptr) {}

  // Converting copy constructor for rebinding
  template <class U>
  ChunkAllocator(const ChunkAllocator<U> &other) noexcept
      : chunk_size_(other.chunk_size_), head_(nullptr), current_(nullptr) {}

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

    // if the allocation is for 0 bytes, return a non-null one byte
    if (n == 0) {
      return static_cast<T *>(detail::non_null_one_byte());
    }

    std::size_t bytes = n * sizeof(T);
    std::size_t align = alignof(T);

    // if current is null, allocate a new chunk
    // set head and current to the new chunk
    if (!current_) {
      std::size_t size = std::max(bytes, chunk_size_);
      current_ = new Chunk(size);
      head_ = current_;
    }

    // allocate from the current chunk
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

  template <typename U, typename... Args> void construct(U *p, Args &&...args) {
    ::new (static_cast<void *>(p)) U(std::forward<Args>(args)...);
  }

  template <typename U> void destroy(U *p) { p->~U(); }

  template <typename U>
  friend constexpr bool operator==(const ChunkAllocator<T> &,
                                   const ChunkAllocator<U> &) noexcept {
    return true;
  }

  template <typename U>
  friend constexpr bool operator!=(const ChunkAllocator<T> &,
                                   const ChunkAllocator<U> &) noexcept {
    return false;
  }
};