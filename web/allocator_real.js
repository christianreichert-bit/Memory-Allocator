let wasmModule = null;

async function loadWasm() {
    try {
        const response = await fetch('allocator_real.wasm');
        const bytes = await response.arrayBuffer();
        const { instance } = await WebAssembly.instantiate(bytes, {
            env: {
                memory: new WebAssembly.Memory({ initial: 256 }),
                abort: (msg, file, line, col) => {
                    console.error(`WASM abort at ${file}:${line}:${col} - ${msg}`);
                }
            }
        });
        
        wasmModule = instance.exports;
        console.log('WebAssembly loaded:', Object.keys(wasmModule));
        return true;
    } catch (error) {
        console.error('Failed to load WebAssembly:', error);
        return false;
    }
}

// Make functions available globally
window.WASM = {
    load: loadWasm,
    initAllocator: (type, size) => wasmModule?._wasm_init_allocator?.(type, size) || 0,
    malloc: (size) => wasmModule?._wasm_malloc?.(size) || null,
    free: (ptr) => wasmModule?._wasm_free?.(ptr),
    getUsedMemory: () => wasmModule?._wasm_get_used_memory?.() || 0,
    getFragmentation: () => wasmModule?._wasm_get_fragmentation?.() || 0,
    cleanup: () => wasmModule?._wasm_cleanup?.(),
    testAllocator: () => wasmModule?._wasm_test_allocator?.() || 0,
    benchmark: (iterations) => wasmModule?._wasm_benchmark?.(iterations) || 0
};
