#include <stdlib.h>

#include "chunk.h"
#include "memory.h"

// State: Initialize chunk to empty
void initChunk(Chunk *chunk) {
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;     // Start empty; allocated on first write 
    chunk->lines = NULL;
    initValueArray(&chunk->constants);
}

// Deallocate memory and reset chunk
void freeChunk(Chunk *chunk) {
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
    FREE_ARRAY(int, chunk->lines, chunk->capacity);
    freeValueArray(&chunk->constants);
    initChunk(chunk);
}

// Appends a byte to the end of the chunk
void writeChunk(Chunk *chunk, uint8_t byte, int line) {
    // Ensure capacity before writing
    if (chunk->capacity < chunk->count + 1) {
        int oldCapacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(oldCapacity);
        chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
        chunk->lines = GROW_ARRAY(int, chunk->lines, oldCapacity, chunk->capacity);
    }

    chunk->code[chunk->count] = byte;
    chunk->lines[chunk->count] = line;
    chunk->count++;
}

// Adds constant to the constant pool; then returns the index where the constant was appended
int addConstant(Chunk *chunk, Value val) {
    writeValueArray(&chunk->constants, val);
    return chunk->constants.count - 1;
}