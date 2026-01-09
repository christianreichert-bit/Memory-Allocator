#include <emscripten.h>
#include <stdlib.h>
#include "../src/allocator.h"

// Global allocator instance
static allocator_t* global_alloc = NULL;

EMSCRIPTEN_KEEPALIVE
int wasm_init_allocator(int type, size_t size) {
    if (global_alloc) {
        destroy_allocator(global_alloc);
    }
    global_alloc = create_allocator(type, size);
    return global_alloc != NULL ? 1 : 0;
}

EMSCRIPTEN_KEEPALIVE
void* wasm_malloc(size_t size) {
    if (!global_alloc) return NULL;
    return global_alloc->malloc(global_alloc, size);
}

EMSCRIPTEN_KEEPALIVE
void wasm_free(void* ptr) {
    if (!global_alloc || !ptr) return;
    global_alloc->free(global_alloc, ptr);
}

EMSCRIPTEN_KEEPALIVE
size_t wasm_get_used_memory() {
    if (!global_alloc) return 0;
    return global_alloc->get_used_memory(global_alloc);
}

EMSCRIPTEN_KEEPALIVE
size_t wasm_get_fragmentation() {
    if (!global_alloc) return 0;
    return global_alloc->get_fragmentation(global_alloc);
}

EMSCRIPTEN_KEEPALIVE
void wasm_cleanup() {
    if (global_alloc) {
        destroy_allocator(global_alloc);
        global_alloc = NULL;
    }
}

// Simple test that proves C code is running
EMSCRIPTEN_KEEPALIVE
int wasm_test_allocator() {
    if (!wasm_init_allocator(0, 1024 * 1024)) return 0;
    
    void* ptr1 = wasm_malloc(100);
    void* ptr2 = wasm_malloc(200);
    
    size_t used = wasm_get_used_memory();
    
    wasm_free(ptr1);
    wasm_free(ptr2);
    wasm_cleanup();
    
    return used > 0 ? 1 : 0;
}

// Direct function for benchmarking
EMSCRIPTEN_KEEPALIVE
int wasm_benchmark(int iterations) {
    wasm_init_allocator(0, 1024 * 1024);
    
    int success = 0;
    for (int i = 0; i < iterations; i++) {
        void* ptr = wasm_malloc((i % 256) + 1);
        if (ptr) {
            success++;
            wasm_free(ptr);
        }
    }
    
    wasm_cleanup();
    return success;
}
