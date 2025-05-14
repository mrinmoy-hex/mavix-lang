#include <stdio.h>

#include "chunk.h"
#include "debug.h"
#include "value.h"

int main(int argc, const char* argv[]) {
    Chunk chunk;
    
    // Initializing the chunk 
    initChunk(&chunk);

    int constant = addConstant(&chunk, 1.2);
    writeChunk(&chunk, OP_CONSTANT, 123);
    writeChunk(&chunk, constant, 123);

    // Writing bytes into chunk
    writeChunk(&chunk, OP_RETURN, 123);

    disassembleChunk(&chunk, "test chunk");

    freeChunk(&chunk);

    return 0;
}