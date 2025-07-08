#pragma once

#include "arena.hpp"

#include <limits>
#include <memory>

namespace detail {
// One byte of storage, maximally aligned.
inline std::aligned_storage_t<1, alignof(std::max_align_t)> dummy;

inline void *non_null_one_byte() noexcept { return &dummy; }
} // namespace detail

template <typename T> class ChunkAllocator {
private:
  std::shared_ptr<Arena> arena_;
  template <typename U> friend class ChunkAllocator;

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
      : arena_(std::make_shared<Arena>(chunk_size)) {}
  ChunkAllocator() : arena_(std::make_shared<Arena>()) {}

  // Copy constructor – makes a fresh allocator that shares no state
  ChunkAllocator(const ChunkAllocator &other) noexcept : arena_(other.arena_) {}

  // Converting copy constructor for rebinding
  template <class U>
  ChunkAllocator(const ChunkAllocator<U> &other) noexcept
      : arena_(other.arena_) {}

  ~ChunkAllocator() = default;

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

    void *ptr = arena_->allocate(bytes, align);

    return static_cast<T *>(ptr);
  }

  // chunk allocator does nothing
  void deallocate(pointer p, std::size_t n) noexcept {}

  size_type max_size() const noexcept {
    return std::numeric_limits<size_type>::max() / sizeof(T);
  }

  void reset() noexcept { arena_->reset(); }

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