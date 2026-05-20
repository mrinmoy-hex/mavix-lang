#ifndef mavix_vm_h
#define mavix_vm_h

#include "chunk.h"
#include "value.h"

#define STACK_MAX 256

// The virtual machine.
typedef struct {
    Chunk *chunk;
    uint8_t* ip;        // Points to the CURRENT INSTRUCTION in code
    Value stack[STACK_MAX];
    Value* stackTop;    // points to the next empty slot in the stack array
} VM;

// The result of interpreting some code.
typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;


void initVM();
void freeVM();

InterpretResult interpret(const char* source);
void push(Value value);
Value pop();

#endif