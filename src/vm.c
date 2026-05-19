#include "common.h"
#include "vm.h"
#include "debug.h"

#include <stdio.h>



VM vm;

void initVM() {

}

void freeVM() {

}

static InterpretResult run() {
    // Reads the byte currently pointed at by the IP and advances the pointer.
#define READ_BYTE() (*vm.ip++)

    // Reads a 1-byte index from the bytecode and looks up the corresponding 
    // constant value from the chunk's constant pool.
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])

    for (;;) {
// Debugging: Disassemble the current instruction before executing it.
#ifdef DEBUG_TRACE_EXEC
        disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
#endif

        uint8_t instruction;
        // Fetch and Decode: Read the opcode and dispatch to the correct case.
        switch (instruction = READ_BYTE()) {
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                printValue(constant);
                printf("\n");
                break;
            }
            case OP_RETURN:
                return INTERPRET_OK;
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
}


InterpretResult interpret(Chunk *chunk) {
    vm.chunk = chunk;
    vm.ip = vm.chunk->code;    // Point IP to the very first bytecode instruction.
    return run();
}