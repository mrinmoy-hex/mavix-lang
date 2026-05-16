#ifndef mavix_vm_h
#define mavix_vm_h

#include "chunk.h"

// The virtual machine.
typedef struct {
    Chunk *chunk;
    uint8_t* ip;    // Instruction pointer
} VM;

// The result of interpreting some code.
typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;


void initVM();
void freeVM();

InterpretResult interpret(Chunk *chunk);

#endif