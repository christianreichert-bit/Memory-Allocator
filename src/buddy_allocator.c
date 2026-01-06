#include "allocator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MIN_BLOCK_SIZE 64  // 64 bytes minimum
#define MAX_LEVELS 12      // 64B -> 128B -> ... -> 64KB

typedef struct buddy_block {
    size_t size;
    int is_free;
    struct buddy_block* next;
} buddy_block_t;

typedef struct {
    void* memory_pool;
    size_t pool_size;
    buddy_block_t* free_lists[MAX_LEVELS];
    size_t used_memory;
    size_t total_memory;
} buddy_allocator_internal_t;

// Helper: Get next power of two
static size_t next_power_of_two(size_t size) {
    if (size < MIN_BLOCK_SIZE) return MIN_BLOCK_SIZE;
    size_t power = 1;
    while (power < size) power <<= 1;
    return power;
}

// Helper: Get level from size
static int get_level(size_t size) {
    int level = 0;
    size_t block_size = MIN_BLOCK_SIZE;
    while (block_size < size) {
        block_size <<= 1;
        level++;
    }
    return level < MAX_LEVELS ? level : MAX_LEVELS - 1;
}

static void* buddy_malloc(allocator_t* alloc, size_t size) {
    buddy_allocator_internal_t* buddy = (buddy_allocator_internal_t*)alloc->internal_data;
    
    if (size == 0) return NULL;
    
    // For now, just use system malloc as placeholder
    // TODO: Implement actual buddy allocation
    printf("[BUDDY] malloc(%zu) - placeholder using system malloc\n", size);
    return malloc(size);
}

static void buddy_free(allocator_t* alloc, void* ptr) {
    if (!ptr) return;
    
    buddy_allocator_internal_t* buddy = (buddy_allocator_internal_t*)alloc->internal_data;
    
    // For now, just use system free as placeholder
    // TODO: Implement actual buddy free
    printf("[BUDDY] free(%p) - placeholder using system free\n", ptr);
    free(ptr);
}

static void* buddy_calloc(allocator_t* alloc, size_t num, size_t size) {
    size_t total = num * size;
    void* ptr = alloc->malloc(alloc, total);
    if (ptr) {
        memset(ptr, 0, total);
    }
    return ptr;
}

static size_t buddy_get_used_memory(allocator_t* alloc) {
    buddy_allocator_internal_t* buddy = (buddy_allocator_internal_t*)alloc->internal_data;
    return buddy ? buddy->used_memory : 0;
}

// Create a buddy allocator instance
allocator_t* create_buddy_allocator(size_t pool_size) {
    // Allocate memory for internal structure
    buddy_allocator_internal_t* buddy = malloc(sizeof(buddy_allocator_internal_t));
    if (!buddy) return NULL;
    
    // Initialize
    buddy->pool_size = next_power_of_two(pool_size);
    buddy->used_memory = 0;
    buddy->total_memory = buddy->pool_size;
    
    // Allocate memory pool
    buddy->memory_pool = malloc(buddy->pool_size);
    if (!buddy->memory_pool) {
        free(buddy);
        return NULL;
    }
    
    // Initialize free lists
    for (int i = 0; i < MAX_LEVELS; i++) {
        buddy->free_lists[i] = NULL;
    }
    
    // Create initial free block (entire pool)
    buddy_block_t* initial_block = (buddy_block_t*)buddy->memory_pool;
    initial_block->size = buddy->pool_size;
    initial_block->is_free = 1;
    initial_block->next = NULL;
    
    int top_level = get_level(buddy->pool_size);
    buddy->free_lists[top_level] = initial_block;
    
    // Create allocator interface
    allocator_t* alloc = malloc(sizeof(allocator_t));
    alloc->malloc = buddy_malloc;
    alloc->free = buddy_free;
    alloc->calloc = buddy_calloc;
    alloc->realloc = NULL; // TODO: Implement
    alloc->get_used_memory = buddy_get_used_memory;
    alloc->get_fragmentation = NULL; // TODO: Implement
    alloc->internal_data = buddy;
    
    printf("[BUDDY] Created allocator with %zu bytes pool\n", buddy->pool_size);
    return alloc;
}

void destroy_buddy_allocator(allocator_t* alloc) {
    if (!alloc) return;
    
    buddy_allocator_internal_t* buddy = (buddy_allocator_internal_t*)alloc->internal_data;
    if (buddy) {
        free(buddy->memory_pool);
        free(buddy);
    }
    free(alloc);
    printf("[BUDDY] Destroyed allocator\n");
}