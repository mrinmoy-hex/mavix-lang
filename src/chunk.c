#include <stdlib.h>

#include "chunk.h"
#include "memory.h"

void initChunk(Chunk* chunk) {
    chunk->count = 0;           // No bytes written yet
    chunk->capacity = 0;        // No memory allocated yet
    chunk->code = NULL;         // Code array not initialized
    initValueArray(&chunk->constants);
}


void freeChunk(Chunk* chunk) {
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
    freeValueArray(&chunk->constants);
    initChunk(chunk);
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

// Method to add a new constant to the chunk and returns the index of the previous constant
int addConstant(Chunk* chunk, Value value) {
    writeValueArray(&chunk->constants, value);
    return chunk->constants.count - 1;
}