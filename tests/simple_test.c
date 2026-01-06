#include "../src/allocator.h"
#include <stdio.h>
#include <time.h>

int main() {
    printf("=== Memory Allocator Demo ===\n\n");
    
    // Test Buddy Allocator
    printf("1. Testing Buddy Allocator:\n");
    allocator_t* buddy = create_allocator(ALLOC_BUDDY, 1024 * 1024);
    if (buddy) {
        int* arr = (int*)buddy->malloc(buddy, sizeof(int) * 100);
        printf("   Allocated 100 integers\n");
        buddy->free(buddy, arr);
        printf("   Freed memory\n");
        printf("   Used memory: %zu bytes\n", buddy->get_used_memory(buddy));
        printf("   Fragmentation: %zu%%\n", buddy->get_fragmentation(buddy));
        destroy_allocator(buddy);
    }
    
    printf("\n2. Testing Slab Allocator:\n");
    allocator_t* slab = create_allocator(ALLOC_SLAB, 1024 * 1024);
    if (slab) {
        void* ptr = slab->malloc(slab, 128);
        printf("   Allocated 128 bytes\n");
        slab->free(slab, ptr);
        printf("   Freed memory\n");
        destroy_allocator(slab);
    }
    
    printf("\n3. Performance Test:\n");
    clock_t start = clock();
    
    allocator_t* perf = create_allocator(ALLOC_BUDDY, 1024 * 1024);
    for (int i = 0; i < 5000; i++) {
        void* p = perf->malloc(perf, (i % 256) + 1);
        perf->free(perf, p);
    }
    destroy_allocator(perf);
    
    double elapsed = (double)(clock() - start) / CLOCKS_PER_SEC;
    printf("   10000 operations in %.3f seconds (%.0f ops/sec)\n", elapsed, 10000 / elapsed);
    
    printf("\n=== Demo Complete ===\n");
    return 0;
}
