# Allocator Strategies
Below are some allocator strategies that I am considering for VortexAlloc.

## Bump Allocator
Bump allocators or stack allocators are simple, and fast. They use on contiguous chunk of memory, and allocate memory from the top or bottom of the chunk; depends if you're bumping up or down. Fress all of memory at once. O(1) allocation and deallocation.

### References
- [Bump Allocation: Up or Down?](https://coredumped.dev/2024/03/25/bump-allocation-up-or-down/)
- [Always Bump Downwards](https://fitzgen.com/2019/11/01/always-bump-downwards.html)
- [Allocator Designs](https://os.phil-opp.com/allocator-designs/#bump-allocator)
- [Arena Allocator Tips and Tricks](https://nullprogram.com/blog/2023/09/27)

## Chunked Bump Allocator
Like a bump allocator, but instead of allocating from a single contiguous chunk of memory, it allocates from a series that are allocated as needed.
Memory still frees all at once, but the allocator can allocate from multiple chunks. O(1) for allocations and O(k) for deallocation where k is the number of chunks.