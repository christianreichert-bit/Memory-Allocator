#include "allocator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MIN_BLOCK_SIZE 64      // 64 bytes minimum
#define MAX_LEVELS 16          // Increased: 64B -> 128B -> ... -> 4MB
#define BUDDY_MAGIC 0xBADB001  // Magic number for validation

typedef struct buddy_block {
    size_t size;
    int is_free;
    unsigned long magic;
    struct buddy_block* next;
    struct buddy_block* prev;
} buddy_block_t;

typedef struct {
    void* memory_pool;
    size_t pool_size;
    buddy_block_t* free_lists[MAX_LEVELS];
    size_t used_memory;
    size_t total_memory;
    size_t total_allocations;
    size_t total_frees;
    size_t failed_allocations;
} buddy_allocator_internal_t;

// Helper: Get next power of two
static size_t next_power_of_two(size_t size) {
    if (size < MIN_BLOCK_SIZE) return MIN_BLOCK_SIZE;
    size--;
    size |= size >> 1;
    size |= size >> 2;
    size |= size >> 4;
    size |= size >> 8;
    size |= size >> 16;
    size |= size >> 32;
    return size + 1;
}

// Helper: Get level from size - FIXED BOUNDS CHECK
static int get_level(size_t size) {
    if (size < MIN_BLOCK_SIZE) return 0;
    
    size_t block_size = MIN_BLOCK_SIZE;
    int level = 0;
    
    while (block_size < size && level < MAX_LEVELS - 1) {
        block_size <<= 1;
        level++;
    }
    
    return level;
}

// Helper: Calculate buddy address
static buddy_block_t* get_buddy(buddy_block_t* block, size_t block_size) {
    uintptr_t addr = (uintptr_t)block;
    uintptr_t buddy_addr = addr ^ block_size;
    return (buddy_block_t*)buddy_addr;
}

static void* buddy_malloc(allocator_t* alloc, size_t size) {
    buddy_allocator_internal_t* buddy = (buddy_allocator_internal_t*)alloc->internal_data;
    
    if (size == 0 || size > buddy->pool_size) {
        buddy->failed_allocations++;
        return NULL;
    }
    
    // Calculate required size with header
    size_t required_size = size + sizeof(buddy_block_t);
    size_t block_size = next_power_of_two(required_size);
    int level = get_level(block_size);
    
    if (level >= MAX_LEVELS) {
        buddy->failed_allocations++;
        return NULL;
    }
    
    // Search for free block at this level or higher
    int search_level = level;
    while (search_level < MAX_LEVELS && buddy->free_lists[search_level] == NULL) {
        search_level++;
    }
    
    if (search_level == MAX_LEVELS) {
        buddy->failed_allocations++;
        return NULL; // No free block found
    }
    
    // Take block from free list
    buddy_block_t* block = buddy->free_lists[search_level];
    buddy->free_lists[search_level] = block->next;
    if (block->next) block->next->prev = NULL;
    
    // Split block down to required level
    while (search_level > level) {
        search_level--;
        size_t half_size = block->size >> 1;
        
        // Create buddy block
        buddy_block_t* buddy_block = (buddy_block_t*)((char*)block + half_size);
        buddy_block->size = half_size;
        buddy_block->is_free = 1;
        buddy_block->magic = BUDDY_MAGIC;
        buddy_block->next = buddy->free_lists[search_level];
        buddy_block->prev = NULL;
        if (buddy_block->next) buddy_block->next->prev = buddy_block;
        buddy->free_lists[search_level] = buddy_block;
        
        // Update current block
        block->size = half_size;
    }
    
    // Mark block as allocated
    block->is_free = 0;
    block->magic = BUDDY_MAGIC;
    block->next = NULL;
    block->prev = NULL;
    
    buddy->used_memory += block->size;
    buddy->total_allocations++;
    
    // Return user pointer (after header)
    void* user_ptr = (void*)((char*)block + sizeof(buddy_block_t));
    
    return user_ptr;
}

static void buddy_free(allocator_t* alloc, void* ptr) {
    if (!ptr) return;
    
    buddy_allocator_internal_t* buddy = (buddy_allocator_internal_t*)alloc->internal_data;
    
    // Get block header
    buddy_block_t* block = (buddy_block_t*)((char*)ptr - sizeof(buddy_block_t));
    
    // Validate magic number
    if (block->magic != BUDDY_MAGIC) {
        return;
    }
    
    if (block->is_free) {
        return;
    }
    
    // Mark as free
    block->is_free = 1;
    buddy->used_memory -= block->size;
    buddy->total_frees++;
    
    // Try to merge with buddy
    int level = get_level(block->size);
    
    while (level < MAX_LEVELS - 1) {
        buddy_block_t* buddy_block = get_buddy(block, block->size);
        
        // Check if buddy is free and valid
        if (buddy_block < (buddy_block_t*)buddy->memory_pool || 
            buddy_block >= (buddy_block_t*)((char*)buddy->memory_pool + buddy->pool_size) ||
            buddy_block->magic != BUDDY_MAGIC || !buddy_block->is_free || 
            buddy_block->size != block->size) {
            break; // Cannot merge
        }
        
        // Remove buddy from free list
        int buddy_level = get_level(buddy_block->size);
        buddy_block_t* current = buddy->free_lists[buddy_level];
        buddy_block_t* prev = NULL;
        
        while (current && current != buddy_block) {
            prev = current;
            current = current->next;
        }
        
        if (current == buddy_block) {
            if (prev) {
                prev->next = buddy_block->next;
            } else {
                buddy->free_lists[buddy_level] = buddy_block->next;
            }
            if (buddy_block->next) {
                buddy_block->next->prev = prev;
            }
        }
        
        // Merge blocks (keep lower address)
        if (block > buddy_block) {
            buddy_block_t* temp = block;
            block = buddy_block;
            buddy_block = temp;
        }
        
        // Clear buddy block
        memset(buddy_block, 0, sizeof(buddy_block_t));
        
        // Double block size
        block->size <<= 1;
        level++;
    }
    
    // Add to free list
    int free_level = get_level(block->size);
    block->next = buddy->free_lists[free_level];
    block->prev = NULL;
    if (block->next) block->next->prev = block;
    buddy->free_lists[free_level] = block;
}

static void* buddy_calloc(allocator_t* alloc, size_t num, size_t size) {
    size_t total = num * size;
    void* ptr = alloc->malloc(alloc, total);
    if (ptr) {
        memset(ptr, 0, total);
    }
    return ptr;
}

static void* buddy_realloc(allocator_t* alloc, void* ptr, size_t size) {
    if (!ptr) return alloc->malloc(alloc, size);
    if (size == 0) {
        alloc->free(alloc, ptr);
        return NULL;
    }
    
    return NULL; // Not implemented for now
}

static size_t buddy_get_used_memory(allocator_t* alloc) {
    buddy_allocator_internal_t* buddy = (buddy_allocator_internal_t*)alloc->internal_data;
    return buddy->used_memory;
}

static size_t buddy_get_fragmentation(allocator_t* alloc) {
    buddy_allocator_internal_t* buddy = (buddy_allocator_internal_t*)alloc->internal_data;
    
    if (buddy->total_memory == 0 || buddy->used_memory == buddy->total_memory) {
        return 0;
    }
    
    size_t free_memory = buddy->total_memory - buddy->used_memory;
    size_t largest_free = 0;
    
    for (int i = 0; i < MAX_LEVELS; i++) {
        buddy_block_t* block = buddy->free_lists[i];
        while (block) {
            if (block->size > largest_free) {
                largest_free = block->size;
            }
            block = block->next;
        }
    }
    
    if (largest_free == 0) return 100;
    if (largest_free >= free_memory) return 0;
    
    return (size_t)((1.0 - (double)largest_free / free_memory) * 100);
}

static void buddy_print_debug_info(allocator_t* alloc) {
    buddy_allocator_internal_t* buddy = (buddy_allocator_internal_t*)alloc->internal_data;
    
    printf("\n=== Buddy Allocator Debug Info ===\n");
    printf("Pool Size: %zu bytes\n", buddy->pool_size);
    printf("Used Memory: %zu bytes (%.1f%%)\n", 
           buddy->used_memory, 
           (double)buddy->used_memory / buddy->pool_size * 100);
    printf("Fragmentation: %zu%%\n", buddy_get_fragmentation(alloc));
    printf("Total Allocations: %zu\n", buddy->total_allocations);
    printf("Total Frees: %zu\n", buddy->total_frees);
    printf("Failed Allocations: %zu\n", buddy->failed_allocations);
    
    printf("\nFree Lists:\n");
    for (int i = 0; i < MAX_LEVELS; i++) {
        size_t block_size = MIN_BLOCK_SIZE << i;
        if (buddy->free_lists[i] != NULL) {
            int count = 0;
            buddy_block_t* block = buddy->free_lists[i];
            while (block) {
                count++;
                block = block->next;
            }
            printf("  Level %2d (%6zu bytes): %d blocks free\n", i, block_size, count);
        }
    }
    printf("===================================\n");
}

allocator_t* create_buddy_allocator(size_t pool_size) {
    // Validate pool size
    if (pool_size < MIN_BLOCK_SIZE) {
        pool_size = MIN_BLOCK_SIZE;
    }
    
    // Allocate internal structure
    buddy_allocator_internal_t* buddy = malloc(sizeof(buddy_allocator_internal_t));
    if (!buddy) {
        return NULL;
    }
    
    // Initialize to zero
    memset(buddy, 0, sizeof(buddy_allocator_internal_t));
    
    // Set pool size
    buddy->pool_size = next_power_of_two(pool_size);
    buddy->total_memory = buddy->pool_size;
    
    // Allocate memory pool
    buddy->memory_pool = malloc(buddy->pool_size);
    if (!buddy->memory_pool) {
        free(buddy);
        return NULL;
    }
    
    // Clear memory pool
    memset(buddy->memory_pool, 0, buddy->pool_size);
    
    // Create initial free block (entire pool)
    buddy_block_t* initial_block = (buddy_block_t*)buddy->memory_pool;
    initial_block->size = buddy->pool_size;
    initial_block->is_free = 1;
    initial_block->magic = BUDDY_MAGIC;
    initial_block->next = NULL;
    initial_block->prev = NULL;
    
    int top_level = get_level(buddy->pool_size);
    if (top_level >= MAX_LEVELS) {
        top_level = MAX_LEVELS - 1;
    }
    
    buddy->free_lists[top_level] = initial_block;
    
    // Create allocator interface
    allocator_t* alloc = malloc(sizeof(allocator_t));
    if (!alloc) {
        free(buddy->memory_pool);
        free(buddy);
        return NULL;
    }
    
    alloc->malloc = buddy_malloc;
    alloc->free = buddy_free;
    alloc->calloc = buddy_calloc;
    alloc->realloc = buddy_realloc;
    alloc->get_used_memory = buddy_get_used_memory;
    alloc->get_fragmentation = buddy_get_fragmentation;
    alloc->print_debug_info = buddy_print_debug_info;
    alloc->internal_data = buddy;
    alloc->type = ALLOC_BUDDY;
    
    return alloc;
}

void destroy_buddy_allocator(allocator_t* alloc) {
    if (!alloc) return;
    
    buddy_allocator_internal_t* buddy = (buddy_allocator_internal_t*)alloc->internal_data;
    if (buddy) {
        if (buddy->memory_pool) {
            free(buddy->memory_pool);
        }
        free(buddy);
    }
    free(alloc);
}