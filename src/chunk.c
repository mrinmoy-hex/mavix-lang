#include <stdlib.h>

#include "chunk.h"
#include "memory.h"


// ============================================================================
// CHUNK INITIALIZATION & CLEANUP
// ============================================================================

/**
 * Initializes a chunk to an empty state
 * Must be called before using a chunk to manage bytecode
 * 
 * Sets up:
 *   - count: number of bytes currently used
 *   - capacity: total allocated space
 *   - code: array of bytecode instructions (NULL initially)
 *   - lines: array of line numbers for error reporting (NULL initially)
 *   - constants: dynamic array to store constant values
 * 
 * @param chunk  Pointer to the Chunk to initialize
 */
void initChunk(Chunk* chunk) {
    chunk->count = 0;           // No bytes written yet
    chunk->capacity = 0;        // No memory allocated yet
    chunk->code = NULL;         // Start empty; allocated on first write
    chunk->lines = NULL;        // Start empty; allocated on first write
    initValueArray(&chunk->constants);  // Initialize the constants pool
}


/**
 * Deallocates all memory used by a chunk and resets it to empty state
 * Should be called when done with a chunk to prevent memory leaks
 * 
 * Frees:
 *   - The bytecode instruction array
 *   - The line number tracking array
 *   - All constant values stored in the pool
 * 
 * Then reinitializes the chunk so it can be reused if needed
 * 
 * @param chunk  Pointer to the Chunk to deallocate
 */
void freeChunk(Chunk* chunk) {
    // Free the bytecode instruction array
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);

    // Free the line number tracking array
    FREE_ARRAY(int, chunk->lines, chunk->capacity);

    // Free all values stored in the constants pool
    freeValueArray(&chunk->constants);

    // Reset the chunk to empty state so it can be reused
    initChunk(chunk);
}


// ============================================================================
// BYTECODE WRITING
// ============================================================================

/**
 * Appends a single byte of bytecode to the end of the chunk
 * Also records the source line number for that byte (for error reporting)
 * 
 * Uses dynamic array growth strategy: when capacity is exceeded,
 * the capacity is increased by GROW_CAPACITY() and arrays are reallocated
 * 
 * Invariants maintained:
 *   - chunk->count: number of bytes written (0-based index of next write)
 *   - chunk->capacity: total allocated space
 *   - chunk->code and chunk->lines: always same length (capacity)
 * 
 * @param chunk  Pointer to the Chunk to write to
 * @param byte  The bytecode instruction or operand byte to append
 * @param line  The source code line number where this byte originated
 */
void writeChunk(Chunk* chunk, uint8_t byte, int line) {
    // Check if we need to grow the arrays to make room for the new byte
    if (chunk->capacity < chunk->count + 1) {
        int oldCapacity = chunk->capacity;

        // Calculate new capacity (typically 2x the old capacity)
        chunk->capacity = GROW_CAPACITY(oldCapacity);

        // Reallocate and copy the bytecode array
        chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);

        // Reallocate and copy the line number array
        chunk->lines = GROW_ARRAY(int, chunk->lines, oldCapacity, chunk->capacity);
    }

    // Write the byte at the current count position
    chunk->code[chunk->count] = byte;

    // Record the source line number for this byte
    chunk->lines[chunk->count] = line;

    // Advance the count
    chunk->count++;
}


// ============================================================================
// CONSTANT POOL MANAGEMENT
// ============================================================================

/**
 * Adds a constant value to the chunk's constant pool
 * Returns the index where the constant was stored
 * 
 * The constant index is used as an operand to OP_CONSTANT instructions:
 *   - The VM executes: OP_CONSTANT <index>
 *   - The index is used to look up the constant in this pool
 *   - The constant's value is pushed onto the VM's stack
 * 
 * Pattern: Multiple OP_CONSTANT instructions can reference the same index,
 * so the constant pool deduplicates identical values (if optimized)
 * 
 * @param chunk  Pointer to the Chunk containing the constant pool
 * @param val  The value to add to the pool (typically a numeric literal)
 * @return  The index of the constant in the pool (0-based)
 */
int addConstant(Chunk* chunk, Value val) {
    // Append the value to the constants array
    writeValueArray(&chunk->constants, val);

    // Return the index where it was stored (count - 1 after append)
    return chunk->constants.count - 1;
}