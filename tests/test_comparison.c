#include "../src/allocator.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>

#define POOL_SIZE (1024 * 1024 * 16) // 16MB
#define NUM_OPERATIONS 100000
#define MAX_ALLOC_SIZE 4096

void benchmark_allocator(allocator_type type, const char* name) {
    printf("\n=== Benchmarking %s ===\n", name);
    
    allocator_t* alloc = NULL;
    if (type != ALLOC_SYSTEM) {
        alloc = create_allocator(type, POOL_SIZE);
        if (!alloc) {
            printf("Failed to create allocator!\n");
            return;
        }
    }
    
    clock_t start = clock();
    void* pointers[NUM_OPERATIONS];
    size_t sizes[NUM_OPERATIONS];
    
    // Allocation phase
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        sizes[i] = (rand() % MAX_ALLOC_SIZE) + 1;
        if (type == ALLOC_SYSTEM) {
            pointers[i] = malloc(sizes[i]);
        } else {
            pointers[i] = ALLOC_MALLOC(alloc, sizes[i]);
        }
        
        if (pointers[i]) {
            memset(pointers[i], i % 256, sizes[i]); // Touch memory
        }
    }
    
    clock_t alloc_time = clock() - start;
    
    // Free phase
    start = clock();
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        if (pointers[i]) {
            if (type == ALLOC_SYSTEM) {
                free(pointers[i]);
            } else {
                ALLOC_FREE(alloc, pointers[i]);
            }
        }
    }
    clock_t free_time = clock() - start;
    
    double total_time = ((double)(alloc_time + free_time)) / CLOCKS_PER_SEC;
    
    printf("Results:\n");
    printf("  Allocation Time: %.3f seconds\n", (double)alloc_time / CLOCKS_PER_SEC);
    printf("  Free Time: %.3f seconds\n", (double)free_time / CLOCKS_PER_SEC);
    printf("  Total Time: %.3f seconds\n", total_time);
    printf("  Operations/sec: %.0f\n", NUM_OPERATIONS * 2 / total_time);
    
    if (alloc) {
        printf("  Memory Used: %zu bytes\n", alloc->get_used_memory(alloc));
        printf("  Fragmentation: %zu%%\n", alloc->get_fragmentation(alloc));
        if (alloc->print_debug_info) {
            alloc->print_debug_info(alloc);
        }
        destroy_allocator(alloc);
    }
}

void test_database_workload() {
    printf("\n=== Database-like Workload ===\n");
    printf("Many small allocations (64-256 bytes)\n");
    
    allocator_type types[] = {ALLOC_BUDDY, ALLOC_SLAB, ALLOC_SYSTEM};
    const char* names[] = {"Buddy", "Slab", "System"};
    
    for (int i = 0; i < 3; i++) {
        benchmark_allocator(types[i], names[i]);
    }
}

void test_mixed_workload() {
    printf("\n=== Mixed Workload ===\n");
    printf("Various allocation sizes (64-4096 bytes)\n");
    
    // Similar implementation...
}

int main() {
    printf("=========================================\n");
    printf("Memory Allocator Benchmark Suite\n");
    printf("=========================================\n");
    
    srand(time(NULL));
    
    test_database_workload();
    test_mixed_workload();
    
    printf("\n=========================================\n");
    printf("Benchmark Complete\n");
    printf("=========================================\n");
    
    return 0;
}