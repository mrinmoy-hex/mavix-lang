//
// Created by mrinmoy on 5/19/25.
//
#include "common.h"
#include "vm.h"
#include "debug.h"
#include "compiler.h"

#include <stdio.h>

VM vm;

static void resetStack() {
    vm.stackTop = vm.stack;
}


void initVM() {
    resetStack();
}

void freeVM() {

}

void push(Value value ) {
    *vm.stackTop = value;
    vm.stackTop++;
}

Value pop() {
    vm.stackTop--;
    return *vm.stackTop;
}


static InterpretResult run() {
    // helper macros
#define READ_BYTE() (*vm.ip++)      // reads the current byte and advances it
/* @note  
 * Uses READ_BYTE() to get the index of a constant in the constants array.
 *
 * @return:
 *  Returns the Value at that index. 
 * */
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])

#define BINARY_OP(op) \
    do { \
        double b = pop(); \
        double a = pop(); \
        push(a op b); \
    } while (false)

    for (;;) {
    
    // For diagnostic logging 
/* 
When this flag is defined, the VM disassembles and prints each instruction right before 
executing it.
*/
#ifdef DEBUG_TRACE_EXECUTION
    // show the current content of the stack
    printf("          ");
    for (Value* slot = vm.stack; slot < vm.stackTop; slot++) {
        printf("[ ");
        printValue(*slot);
        printf(" ]");
    }
    printf("\n");
    disassembleInstruction(vm.chunk, (int) (vm.ip - vm.chunk->code ));  // computes the current offset
#endif 

        uint8_t instruction;
        switch (instruction = READ_BYTE()) {
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                push(constant);
                break;
            }
            case OP_ADD:        BINARY_OP(+); break;
            case OP_SUBTRACT:   BINARY_OP(-); break;
            case OP_MULTIPLY:   BINARY_OP(*); break;
            case OP_DIVIDE:     BINARY_OP(/); break;

            /*
            @note 
            The instruction needs a value to operate on, which it gets by popping from the 
            stack. It negates that, then pushes the result back on for later instructions to use.
            */
            case OP_NEGATE:     push(-pop()); break;

            case OP_RETURN: {
                printValue(pop());
                printf("\n");
                return INTERPRET_OK;
            }
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}



InterpretResult interpret(const char* source) {
    compile(source);
    return INTERPRET_OK;
}
