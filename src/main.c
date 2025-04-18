#include <stdio.h>

#include "chunk.h"
#include "debug.h"

int main(int argc, const char* argv[]) {
    Chunk chunk;
    
    // Initializing the chunk 
    initChunk(&chunk);

    // Writing bytes into chunk
    writeChunk(&chunk, OP_RETURN);

    disassembleChunk(&chunk, "test chunk");

    freeChunk(&chunk);

    return 0;
}