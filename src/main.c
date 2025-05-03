#include <stdio.h>

#include "chunk.h"
#include "debug.h"
#include "value.h"

int main(int argc, const char* argv[]) {
    Chunk chunk;
    
    // Initializing the chunk 
    initChunk(&chunk);

    int constant = addConstant(&chunk, 1.2);
    writeChunk(&chunk, OP_CONSTANT);
    writeChunk(&chunk, constant);

    // Writing bytes into chunk
    writeChunk(&chunk, OP_RETURN);

    disassembleChunk(&chunk, "test chunk");

    freeChunk(&chunk);

    return 0;
}