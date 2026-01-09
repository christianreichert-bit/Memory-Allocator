#include <emscripten.h>
#include <stdlib.h>
#include <string.h>
#include "../src/allocator.h"

// Global allocator instance
static allocator_t* g_allocator = NULL;

EMSCRIPTEN_KEEPALIVE
int init_allocator(int type, size_t pool_size) {
    if (g_allocator) {
        destroy_allocator(g_allocator);
    }
    g_allocator = create_allocator(type, pool_size);
    return g_allocator != NULL ? 1 : 0;
}

EMSCRIPTEN_KEEPALIVE
void* allocate_memory(size_t size) {
    if (!g_allocator) return NULL;
    return g_allocator->malloc(g_allocator, size);
}

EMSCRIPTEN_KEEPALIVE
void free_memory(void* ptr) {
    if (!g_allocator || !ptr) return;
    g_allocator->free(g_allocator, ptr);
}

EMSCRIPTEN_KEEPALIVE
size_t get_used_memory() {
    if (!g_allocator) return 0;
    return g_allocator->get_used_memory(g_allocator);
}

EMSCRIPTEN_KEEPALIVE
size_t get_fragmentation() {
    if (!g_allocator) return 0;
    return g_allocator->get_fragmentation(g_allocator);
}

EMSCRIPTEN_KEEPALIVE
void cleanup() {
    if (g_allocator) {
        destroy_allocator(g_allocator);
        g_allocator = NULL;
    }
}

// Test function
EMSCRIPTEN_KEEPALIVE
int test_allocator() {
    init_allocator(0, 1024 * 1024); // ALLOC_BUDDY = 0
    void* ptr = allocate_memory(100);
    if (!ptr) return 0;
    
    size_t used = get_used_memory();
    free_memory(ptr);
    cleanup();
    
    return used > 0 ? 1 : 0;
}
