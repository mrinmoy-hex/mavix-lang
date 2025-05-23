#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
#include "value.h"

typedef enum {
    OP_CONSTANT,
    OP_RETURN,
} OpCode;


typedef struct {
    int count;          // Array element count
    int capacity;       // Number of allocated entried in use
    uint8_t* code;      // Pointer to the array of bytecode instructions
    int* lines;
    ValueArray constants;
} Chunk;


// Initializes a Chunk structure (allocates initial memory, sets counts to zero, etc.)
void initChunk(Chunk* chunk);


// Freeing the memory
void freeChunk(Chunk* chunk);

// Appends a byte to the Chunk's code array, resizing if necessary
void writeChunk(Chunk* chunk, uint8_t byte, int line);

int addConstant(Chunk* chunk, Value value);

#endif