#include <emscripten.h>
#include <stdlib.h>
#include <string.h>

// Simple test functions first
EMSCRIPTEN_KEEPALIVE
int add_numbers(int a, int b) {
    return a + b;
}

EMSCRIPTEN_KEEPALIVE
char* get_hello_message() {
    char* msg = malloc(50);
    strcpy(msg, "Hello from WebAssembly Memory Allocator!");
    return msg;
}

EMSCRIPTEN_KEEPALIVE
void free_string(char* str) {
    free(str);
}