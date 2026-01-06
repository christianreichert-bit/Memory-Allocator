# Memory Allocator Project

A custom memory allocator implementation in C with multiple allocation strategies.

## Features
- **Buddy Allocator**: Power-of-two block allocation with merging
- **Slab Allocator**: Fixed-size allocation for object pools
- **Performance Benchmarking**: Compare against system malloc
- **Visual Debugging**: Heap visualization and fragmentation analysis

## Build and Run
```bash
# Clone and build
make

# Run benchmarks
make test

# Run with debug output
make debug && ./allocator_test