#ifndef mavix_debug_h
#define mavix_debug_h

#include "chunk.h"

// Disassembles an entire chunk of bytecode for debugging purposes.
/** 
    * @param 'chunk' is a pointer to the Chunk to disassemble.
    * @param 'name' is a label used when printing the disassembled chunk.
*/
void disassembleChunk(Chunk* chunk, const char* name);

// Disassembles a single instruction at the given offset in the chunk.
/** 
    * @brief Useful for stepping through bytecode instruction by instruction.
    * @return The offset of the next instruction.
*/
int disassembleInstruction(Chunk* chunk, int offset);


#endif  // mavix_debug_h