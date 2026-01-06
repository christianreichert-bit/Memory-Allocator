#include "../src/allocator.h"
#include <stdio.h>
#include <time.h>
#include <assert.h>

void test_basic_allocation() {
    printf("=== Test 1: Basic Allocation ===\n");
    
    allocator_t* alloc = create_allocator(ALLOC_BUDDY, 1024 * 1024); // 1MB
    assert(alloc != NULL);
    
    // Test 1: Simple malloc/free
    int* ptr = (int*)ALLOC_MALLOC(alloc, sizeof(int) * 100);
    assert(ptr != NULL);
    *ptr = 42;
    assert(*ptr == 42);
    ALLOC_FREE(alloc, ptr);
    
    // Test 2: Multiple allocations
    void* pointers[10];
    for (int i = 0; i < 10; i++) {
        pointers[i] = ALLOC_MALLOC(alloc, 128);
        assert(pointers[i] != NULL);
    }
    
    // Free every other pointer
    for (int i = 0; i < 10; i += 2) {
        ALLOC_FREE(alloc, pointers[i]);
    }
    
    // Allocate more
    for (int i = 0; i < 5; i++) {
        void* p = ALLOC_MALLOC(alloc, 256);
        assert(p != NULL);
        ALLOC_FREE(alloc, p);
    }
    
    // Clean up remaining
    for (int i = 1; i < 10; i += 2) {
        ALLOC_FREE(alloc, pointers[i]);
    }
    
    destroy_allocator(alloc);
    printf("✓ Basic tests passed!\n\n");
}

void test_performance() {
    printf("=== Test 2: Performance ===\n");
    
    allocator_t* alloc = create_allocator(ALLOC_BUDDY, 1024 * 1024); // 1MB
    assert(alloc != NULL);
    
    clock_t start = clock();
    
    const int NUM_ALLOCS = 10000;
    void* pointers[NUM_ALLOCS];
    
    // Allocate many small blocks
    for (int i = 0; i < NUM_ALLOCS; i++) {
        pointers[i] = ALLOC_MALLOC(alloc, 64 + (i % 128));
        assert(pointers[i] != NULL);
    }
    
    // Free half of them
    for (int i = 0; i < NUM_ALLOCS; i += 2) {
        ALLOC_FREE(alloc, pointers[i]);
    }
    
    // Allocate different sizes
    for (int i = 0; i < NUM_ALLOCS / 2; i++) {
        void* p = ALLOC_MALLOC(alloc, 256);
        assert(p != NULL);
        ALLOC_FREE(alloc, p);
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Performance: %d operations in %.3f seconds (%.0f ops/sec)\n",
           NUM_ALLOCS * 2, elapsed, (NUM_ALLOCS * 2) / elapsed);
    
    // Clean up
    for (int i = 1; i < NUM_ALLOCS; i += 2) {
        ALLOC_FREE(alloc, pointers[i]);
    }
    
    destroy_allocator(alloc);
    printf("✓ Performance test completed!\n\n");
}

void test_calloc() {
    printf("=== Test 3: Calloc (Zero-initialized memory) ===\n");
    
    allocator_t* alloc = create_allocator(ALLOC_BUDDY, 1024 * 1024);
    assert(alloc != NULL);
    
    if (alloc->calloc) {
        int* ptr = (int*)alloc->calloc(alloc, 100, sizeof(int));
        assert(ptr != NULL);
        
        // Verify memory is zeroed
        for (int i = 0; i < 100; i++) {
            assert(ptr[i] == 0);
        }
        
        ALLOC_FREE(alloc, ptr);
        printf("✓ Calloc test passed!\n");
    } else {
        printf("⚠ Calloc not implemented\n");
    }
    
    destroy_allocator(alloc);
    printf("\n");
}

int main() {
    printf("=== Memory Allocator Test Suite ===\n\n");
    
    test_basic_allocation();
    test_performance();
    test_calloc();
    
    printf("=== All tests completed successfully! ===\n");
    return 0;
}