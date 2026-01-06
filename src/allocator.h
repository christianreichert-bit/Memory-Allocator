#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h>
#include <stdbool.h>

// Base allocator interface
typedef struct allocator_t allocator_t;

struct allocator_t {
    // Core allocation functions
    void* (*malloc)(allocator_t* alloc, size_t size);
    void (*free)(allocator_t* alloc, void* ptr);
    void* (*calloc)(allocator_t* alloc, size_t num, size_t size);
    void* (*realloc)(allocator_t* alloc, void* ptr, size_t size);
    
    // Diagnostic functions
    size_t (*get_used_memory)(allocator_t* alloc);
    size_t (*get_fragmentation)(allocator_t* alloc);
    void (*print_debug_info)(allocator_t* alloc);
    
    // Internal data
    void* internal_data;
    int type;
};

// Allocator types
typedef enum {
    ALLOC_BUDDY,
    ALLOC_SLAB,
    ALLOC_FREELIST,
    ALLOC_SYSTEM  // For comparison
} allocator_type;

// Public API
allocator_t* create_allocator(allocator_type type, size_t pool_size);
void destroy_allocator(allocator_t* alloc);
const char* get_allocator_name(allocator_type type);

// Convenience macros
#define ALLOC_MALLOC(alloc, size) ((alloc)->malloc((alloc), (size)))
#define ALLOC_FREE(alloc, ptr) ((alloc)->free((alloc), (ptr)))
#define ALLOC_CALLOC(alloc, num, size) ((alloc)->calloc((alloc), (num), (size)))

// Benchmark structure
typedef struct {
    double malloc_time;
    double free_time;
    size_t peak_memory;
    size_t fragmentation;
    size_t operations;
    size_t memory_used;
} benchmark_result_t;

#endif