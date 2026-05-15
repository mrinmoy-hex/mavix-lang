#include <stdlib.h>

#include "chunk.h"
#include "memory.h"

// State: Initialize chunk to empty
void initChunk(Chunk *chunk) {
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;     // Start empty; allocated on first write 
}

// Deallocate memory and reset chunk
void freeChunk(Chunk *chunk) {
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
    initChunk(chunk);
}

// Appends a byte to the end of the chunk
void writeChunk(Chunk *chunk, uint8_t byte) {
    // Ensure capacity before writing
    if (chunk->capacity < chunk->count + 1) {
        int oldCapacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(oldCapacity);
        chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
    }

    chunk->code[chunk->count] = byte;
    chunk->count++;
}