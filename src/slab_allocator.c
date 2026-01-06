#include "allocator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define SLAB_MAGIC 0x51AB001  // Fixed hex number
#define MAX_SLAB_SIZES 8
#define DEFAULT_SLAB_SIZES {64, 128, 256, 512, 1024, 2048, 4096, 8192}

typedef struct slab_block {
    struct slab_block* next;
    unsigned long magic;
    int is_free;
} slab_block_t;

typedef struct slab_cache {
    size_t block_size;
    size_t blocks_per_slab;
    slab_block_t* free_list;
    struct slab_cache* next;
} slab_cache_t;

typedef struct {
    slab_cache_t* caches;
    size_t total_memory;
    size_t used_memory;
    size_t slab_sizes[MAX_SLAB_SIZES];
    int num_sizes;
} slab_allocator_internal_t;

static slab_cache_t* find_or_create_cache(slab_allocator_internal_t* slab, size_t size) {
    // Find appropriate cache
    slab_cache_t* cache = slab->caches;
    slab_cache_t* prev = NULL;
    
    while (cache) {
        if (cache->block_size >= size) {
            return cache;
        }
        prev = cache;
        cache = cache->next;
    }
    
    // Create new cache
    cache = malloc(sizeof(slab_cache_t));
    if (!cache) return NULL;
    
    cache->block_size = size;
    cache->blocks_per_slab = 4096 / size; // One page per slab
    if (cache->blocks_per_slab < 4) cache->blocks_per_slab = 4;
    cache->free_list = NULL;
    cache->next = NULL;
    
    // Add to cache list
    if (prev) {
        prev->next = cache;
    } else {
        slab->caches = cache;
    }
    
    return cache;
}

static void* slab_malloc(allocator_t* alloc, size_t size) {
    slab_allocator_internal_t* slab = (slab_allocator_internal_t*)alloc->internal_data;
    
    if (size == 0) return NULL;
    
    slab_cache_t* cache = find_or_create_cache(slab, size);
    if (!cache) return NULL;
    
    // Allocate new slab if needed
    if (!cache->free_list) {
        size_t slab_size = cache->block_size * cache->blocks_per_slab;
        char* memory = malloc(slab_size);
        if (!memory) return NULL;
        
        slab->total_memory += slab_size;
        
        // Create free list for new slab
        for (size_t i = 0; i < cache->blocks_per_slab; i++) {
            slab_block_t* block = (slab_block_t*)(memory + i * cache->block_size);
            block->next = cache->free_list;
            block->magic = SLAB_MAGIC;
            block->is_free = 1;
            cache->free_list = block;
        }
    }
    
    // Take block from free list
    slab_block_t* block = cache->free_list;
    cache->free_list = block->next;
    
    block->is_free = 0;
    slab->used_memory += cache->block_size;
    
    return (void*)((char*)block + sizeof(slab_block_t));
}

static void slab_free(allocator_t* alloc, void* ptr) {
    if (!ptr) return;
    
    slab_allocator_internal_t* slab = (slab_allocator_internal_t*)alloc->internal_data;
    slab_block_t* block = (slab_block_t*)((char*)ptr - sizeof(slab_block_t));
    
    if (block->magic != SLAB_MAGIC) {
        fprintf(stderr, "[SLAB] ERROR: Invalid pointer!\n");
        return;
    }
    
    // Find which cache this block belongs to
    slab_cache_t* cache = slab->caches;
    while (cache) {
        // Check if block aligns with cache (simplified)
        if (1) { // In real implementation, track which slabs belong to which cache
            block->next = cache->free_list;
            block->is_free = 1;
            cache->free_list = block;
            slab->used_memory -= cache->block_size;
            return;
        }
        cache = cache->next;
    }
    
    fprintf(stderr, "[SLAB] ERROR: Could not find cache for block!\n");
}

// [Similar implementations for other functions - simplified for brevity]
static void* slab_calloc(allocator_t* alloc, size_t num, size_t size) {
    size_t total = num * size;
    void* ptr = alloc->malloc(alloc, total);
    if (ptr) memset(ptr, 0, total);
    return ptr;
}

static size_t slab_get_used_memory(allocator_t* alloc) {
    slab_allocator_internal_t* slab = (slab_allocator_internal_t*)alloc->internal_data;
    return slab->used_memory;
}

static size_t slab_get_fragmentation(allocator_t* alloc) {
    return 0; // Slab allocator has minimal fragmentation
}
void destroy_slab_allocator(allocator_t* alloc) {
    if (!alloc) return;
    
    slab_allocator_internal_t* slab = (slab_allocator_internal_t*)alloc->internal_data;
    if (slab) {
        slab_cache_t* cache = slab->caches;
        while (cache) {
            slab_cache_t* next = cache->next;
            free(cache);
            cache = next;
        }
        free(slab);
    }
    free(alloc);
}

allocator_t* create_slab_allocator(size_t pool_size) {
    slab_allocator_internal_t* slab = malloc(sizeof(slab_allocator_internal_t));
    if (!slab) return NULL;
    
    slab->caches = NULL;
    slab->total_memory = 0;
    slab->used_memory = 0;
    
    // Set default slab sizes
    size_t default_sizes[] = DEFAULT_SLAB_SIZES;
    slab->num_sizes = MAX_SLAB_SIZES;
    memcpy(slab->slab_sizes, default_sizes, sizeof(default_sizes));
    
    allocator_t* alloc = malloc(sizeof(allocator_t));
    alloc->malloc = slab_malloc;
    alloc->free = slab_free;
    alloc->calloc = slab_calloc;
    alloc->realloc = NULL; // TODO
    alloc->get_used_memory = slab_get_used_memory;
    alloc->get_fragmentation = slab_get_fragmentation;
    alloc->print_debug_info = NULL; // TODO
    alloc->internal_data = slab;
    alloc->type = ALLOC_SLAB;
    
    return alloc;
}