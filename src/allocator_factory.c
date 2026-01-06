#include "allocator.h"
#include <stdio.h>
#include <stdlib.h>

// Forward declarations
allocator_t* create_buddy_allocator(size_t pool_size);
void destroy_buddy_allocator(allocator_t* alloc);

allocator_t* create_allocator(allocator_type type, size_t pool_size) {
    switch (type) {
        case ALLOC_BUDDY:
            return create_buddy_allocator(pool_size);
        case ALLOC_SLAB:
            printf("[FACTORY] Slab allocator not implemented yet\n");
            return NULL;
        case ALLOC_FREELIST:
            printf("[FACTORY] Free-list allocator not implemented yet\n");
            return NULL;
        default:
            printf("[FACTORY] Unknown allocator type: %d\n", type);
            return NULL;
    }
}

void destroy_allocator(allocator_t* alloc) {
    if (!alloc) return;
    
    // For now, assume it's a buddy allocator
    // TODO: Add type field to allocator_t to handle different types
    destroy_buddy_allocator(alloc);
}