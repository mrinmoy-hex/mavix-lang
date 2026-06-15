#include "common.h"
#include "vm.h"
#include "debug.h"
#include "compiler.h"

#include <stdarg.h>
#include <stdio.h>

/*
    VM of Mavix
*/

VM vm;

static void resetStack() {
    vm.stackTop = vm.stack;     // point to very beginning
}


/*
Prints an error message to stderr and resets the stack. 
The function takes a format string and a variable number of arguments, similar to printf. 
It uses va_list to handle the variable arguments, formats the error message, and prints it to stderr. It also prints the line number of the instruction that caused the error, which is calculated by subtracting the current instruction pointer (vm.ip) from the start of the chunk's code and looking up the corresponding line number in the chunk's lines array.
 Finally, it calls resetStack() to clear the stack.
*/ 
static void runtimeError(const char* format, ...) {
    va_list args;       // pass arbitrary number of args
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    size_t instruction = vm.ip - vm.chunk->code - 1;    // -1 because the interpreter advances past each instruction before executing it
    int line = vm.chunk->lines[instruction];
    fprintf(stderr, "[line %d] in script\n", line);
    resetStack();
}


void initVM() {
    resetStack();
}

void freeVM() {

}

// Stack Operations
void push(Value value) {
    *vm.stackTop = value;
    vm.stackTop++;
}

Value pop() {
    vm.stackTop--;
    return *vm.stackTop;
}

static Value peek(int distance) {
    return vm.stackTop[-1 - distance];  // -1 because stackTop points to the next empty slot in the stack array
}



static InterpretResult run() {
    // Reads the byte currently pointed at by the IP and advances the pointer.
#define READ_BYTE() (*vm.ip++)

    // Reads a 1-byte index from the bytecode and looks up the corresponding 
    // constant value from the chunk's constant pool.
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])

#define BINARY_OP(valueType, op) \
    do { \
        if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(0))) { \
            runtimeError("Operands must be numbers.");  \
            return INTERPRET_RUNTIME_ERROR; \
        }   \
        double b = AS_NUMBER(pop()); \
        double a = AS_NUMBER(pop()); \
        push(valueType(a op b)); \
    } while(false)

    for (;;) {
// Debugging: Disassemble the current instruction before executing it.
#ifdef DEBUG_TRACE_EXEC
        printf("          ");
        for (Value* slot = vm.stack; slot < vm.stackTop; slot++) 
        {
            printf("[ ");
            printValue(*slot);
            printf(" ]");
        }
        printf("\n");

        disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
#endif

        uint8_t instruction;
        // Fetch and Decode: Read the opcode and dispatch to the correct case.
        switch (instruction = READ_BYTE()) {
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                push(constant);
                break;
            }
            case OP_NIL:            push(NIL_VAL); break;
            case OP_TRUE:           push(BOOL_VAL(true)); break;
            case OP_FALSE:          push(BOOL_VAL(false)); break;
            case OP_ADD:            BINARY_OP(NUMBER_VAL, +); break;
            case OP_SUBTRACT:       BINARY_OP(NUMBER_VAL, -); break;
            case OP_MULTIPLY:       BINARY_OP(NUMBER_VAL, *); break;
            case OP_DIVIDE:         BINARY_OP(NUMBER_VAL, /); break;
            case OP_NEGATE:         // unary negation 
                // check value on top of stack is a number
                if (!IS_NUMBER(peek(0))) {
                    runtimeError("Operand must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(NUMBER_VAL(-AS_NUMBER(pop())));
                break;

            case OP_RETURN:
                printValue(pop());
                printf("\n");
                return INTERPRET_OK;
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}


InterpretResult interpret(const char *source) {
    Chunk chunk;
    initChunk(&chunk);

    if (!compile(source, &chunk)) {
        freeChunk(&chunk);
        return INTERPRET_COMPILE_ERROR;
    }

    vm.chunk = &chunk;
    vm.ip = vm.chunk->code;

    InterpretResult result = run();

    freeChunk(&chunk);
    return result;
}