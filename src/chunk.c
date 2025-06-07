#include <stdlib.h>

#include "chunk.h"
#include "memory.h"

// initialize the chunk with empty array
void initChunk(Chunk* chunk) {
    chunk->count = 0;           // No bytes written yet
    chunk->capacity = 0;        // No memory allocated yet
    chunk->code = NULL;         // Code array not initialized
    chunk->lines = NULL;
    initValueArray(&chunk->constants);
}

// free the chunk and initialize it
void freeChunk(Chunk* chunk) {
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
    FREE_ARRAY(int, chunk->lines, chunk->capacity);
    freeValueArray(&chunk->constants);
    initChunk(chunk);
}

/**
 * @brief Writes a byte to the chunk.
 *
 * This function appends a byte to the chunk's bytecode array and records the line number
 * where the byte was added. It is used to build up the bytecode for the virtual machine.
 *
 * @param chunk A pointer to the Chunk structure where the byte will be written.
 * @param byte The byte value to be written to the chunk.
 * @param line The line number in the source code where this byte was generated.
 */
void writeChunk(Chunk* chunk, uint8_t byte, int line) {
    // Check if there's enough space to add a new byte
    if (chunk->capacity < chunk->count + 1) {
        int oldCapacity = chunk->capacity;
        // Grow capacity (usually doubles)
        chunk->capacity = GROW_CAPACITY(oldCapacity);
        // Reallocate memory with the new capacity
        chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
    
        chunk->lines = GROW_ARRAY(int, chunk->lines, oldCapacity, chunk->capacity);
    }

    // Write the byte and increment count
    chunk->code[chunk->count] = byte;
    chunk->lines[chunk->count] = line;
    chunk->count++;
}

/**
 * Adds a constant value to the constants array in the given chunk.
 * 
 * This function appends the provided constant value to the `constants` array
 * within the `Chunk` and returns the index of the newly added constant.
 * 
 * - The function uses `writeValueArray` to handle the dynamic resizing
 *   of the `constants` array if needed.
 * - The returned index is calculated as `count - 1` because the `count` field
 *   represents the total number of elements in the array after the new value
 *   is added, and array indexing starts at 0.
 * 
 * @param chunk A pointer to the `Chunk` to which the constant is being added.
 * @param value The constant value to add (of type `Value`).
 * @return The index of the newly added constant in the `constants` array.
 */
int addConstant(Chunk* chunk, Value value) {
    writeValueArray(&chunk->constants, value);
    return chunk->constants.count - 1;
}