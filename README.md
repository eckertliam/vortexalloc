# Vortex Allocator

Modern C++ applications often suffer from performance bottlenecks due to inefficient memory allocation patterns, especially in high-frequency allocation scenarios like AST construction or container-heavy systems.

To address this, I built Vortex Allocator a high-performance arena allocator optimized for speed and cache locality. It features chunked linear allocation, shared state across rebinds for STL compatibility, and alignment-aware allocation with near-zero overhead.

In benchmarks using Catch2, Vortex outperformed `std::allocator` by 5–10% in small object workloads (e.g., `std::vector<int>`, `std::list<char>`) and reduced heap fragmentation, making it ideal for compilers, parsers, and real-time systems.
