#ifndef mavix_debug_h
#define mavix_debug_h

#include "chunk.h"

// Disassembles all the instructions in the entire chunk
void disassembleChunk(Chunk *chunk, const char* name);
// Disassembles instruction at given offset; returns offset of next instruction
int disassembleInstruction(Chunk *chunk, int offset);

#endif