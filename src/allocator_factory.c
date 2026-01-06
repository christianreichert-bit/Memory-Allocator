#include "allocator.h"
#include <stdio.h>
#include <stdlib.h>

// Forward declarations - ONLY what we have
allocator_t* create_buddy_allocator(size_t pool_size);
allocator_t* create_slab_allocator(size_t pool_size);
// freelist_allocator not implemented yet

void destroy_buddy_allocator(allocator_t* alloc);
void destroy_slab_allocator(allocator_t* alloc);

allocator_t* create_allocator(allocator_type type, size_t pool_size) {
    switch (type) {
        case ALLOC_BUDDY:
            return create_buddy_allocator(pool_size);
        case ALLOC_SLAB:
            return create_slab_allocator(pool_size);
        case ALLOC_FREELIST:
            printf("[FACTORY] Free-list allocator not implemented yet (returning NULL)\n");
            return NULL;
        case ALLOC_SYSTEM:
            return NULL; // System allocator
        default:
            printf("[FACTORY] Unknown allocator type: %d\n", type);
            return NULL;
    }
}

void destroy_allocator(allocator_t* alloc) {
    if (!alloc) return;
    
    switch (alloc->type) {
        case ALLOC_BUDDY:
            destroy_buddy_allocator(alloc);
            break;
        case ALLOC_SLAB:
            destroy_slab_allocator(alloc);
            break;
        case ALLOC_FREELIST:
            printf("[FACTORY] Warning: Cannot destroy unimplemented free-list allocator\n");
            free(alloc);
            break;
        case ALLOC_SYSTEM:
            free(alloc);
            break;
        default:
            printf("[FACTORY] Cannot destroy unknown allocator type: %d\n", alloc->type);
            break;
    }
}

const char* get_allocator_name(allocator_type type) {
    switch (type) {
        case ALLOC_BUDDY: return "Buddy Allocator";
        case ALLOC_SLAB: return "Slab Allocator";
        case ALLOC_FREELIST: return "Free-list Allocator (not implemented)";
        case ALLOC_SYSTEM: return "System Allocator (malloc/free)";
        default: return "Unknown Allocator";
    }
}