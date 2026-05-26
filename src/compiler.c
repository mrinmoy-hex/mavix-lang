#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "compiler.h"
#include "scanner.h"

// Tracks parsing state and error management flags
typedef struct {
    Token current;
    Token previous;
    bool hadError;    // Set true globally if any code compilation error occurs
    bool panicMode;   // Set true during an error cascade to suppress spam errors
} Parser;

Parser parser;
Chunk* compilingChunk; // The active bytecode array being filled by the compiler

// Returns the active chunk; encapsulated to seamlessly support nested functions later
static Chunk* currentChunk() {
    return compilingChunk;
}


// Low-level diagnostic reporter; formats and prints syntax errors to stderr
static void errorAt(Token* token, const char* message) {
    if (parser.panicMode) return; // Suppress cascade errors until syntax syncs up
    parser.panicMode = true;
    fprintf(stderr, "[line %d] Error", token->line);

    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    }
    else if (token->type == TOKEN_ERROR) {
        // Nothing; the error token's raw text contains the error message string
    }
    else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    parser.hadError = true;
}

// Reports an error on the token the parser has just validated/passed
static void error(const char* message) {
    errorAt(&parser.previous, message);
}

// Reports an error on the upcoming token the parser is currently looking at
static void errorAtCurrent(const char* message) {
    errorAt(&parser.current, message);
}

// Steps forward through the token stream, automatically intercepting scanner errors
static void advance() {
    parser.previous = parser.current;

    for (;;) {
        parser.current = scanToken();
        if (parser.current.type != TOKEN_ERROR) break;

        errorAtCurrent(parser.current.start);
    }
}

// Asserts the next token matches 'type'; if true, advances, else triggers a parsing error
static void consume(TokenType type, const char* message) {
    if (parser.current.type == type) {
        advance();
        return;
    }

    errorAtCurrent(message);
}


// Appends a single byte instruction to the active chunk, tracked to the previous line
static void emitByte(uint8_t byte) {
    writeChunk(currentChunk(), byte, parser.previous.line);
}

// Utility shortcut to write an opcode and its immediate operand byte consecutively
static void emitBytes(uint8_t byte1, uint8_t byte2) {
    emitByte(byte1);
    emitByte(byte2);
}

// Formats the terminating bytecode return signal
static void emitReturn() {
    emitByte(OP_RETURN);
}

static void emitConstant(Value value) {
    emitBytes(OP_CONSTANT, makeConstant(value));
}

// Wraps up compilation tasks once code processing is complete
static void endCompiler() {
    emitReturn();
}

// compile number literals
static void number() {
    double value = strtod(parser.previous.start, NULL);
    emitConstant(value);
}


static void expression() {
    // What goes here?
}


// Orchestrates execution; parses a source code string and populates the given bytecode chunk
bool compile(const char* source, Chunk* chunk) {
    initScanner(source);
    compilingChunk = chunk;

    parser.hadError = false;
    parser.panicMode = false;

    advance(); // Prime the pump by fetching the first valid token

    expression();
    consume(TOKEN_EOF, "Expect end of expression.");

    endCompiler();

    return !parser.hadError; // Returns false if any syntax bugs aborted compilation safety
}