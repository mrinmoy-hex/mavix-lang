#include <stdlib.h>

#include "chunk.h"
#include "memory.h"

void initChunk(Chunk* chunk) {
    chunk->count = 0;           // No bytes written yet
    chunk->capacity = 0;        // No memory allocated yet
    chunk->code = NULL;         // Code array not initialized
}



void writeChunk(Chunk* chunk, uint8_t byte) {
    // Check if there's enough space to add a new byte
    if (chunk->capacity < chunk->count + 1) {
        int oldCapacity = chunk->capacity;
        // Grow capacity (usually doubles)
        chunk->capacity = GROW_CAPACITY(oldCapacity);
        // Reallocate memory with the new capacity
        chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
    }

    // Write the byte and increment count
    chunk->code[chunk->count] = byte;
    chunk->count++;
}