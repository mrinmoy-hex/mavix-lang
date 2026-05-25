#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "compiler.h"
#include "scanner.h"


typedef struct {
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;
} Parser;

Parser parser;


// Core error reporting function; formats and prints compilation errors
static void errorAt(Token* token, const char* message) {
    // suppress any other errors that get detected
    if (parser.panicMode) return;
    parser.panicMode = true;
    fprintf(stderr, "[line %d] Error", token->line);

    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    }
    else if (token->type == TOKEN_ERROR) {
        // nothing.
    }
    else {
        fprintf(stderr, "at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    parser.hadError = true;
}

// Reports an error at the location of the most recently consumed token
static void error(const char* message) {
    errorAt(&parser.previous, message);
}

// Reports an error at the location of the token about to be consumed
static void errorAtCurrent(count char* message) {
    errorAt(&parser.current, message);
}

// Step forward through the token stream
static void advance() {
    parser.previous = parser.current;

    for (;;) {
        parser.current = scanToken();
        if (parser.current.type != TOKEN_ERROR) break;

        errorAtCurrent(parser.current.start);
    }
}

static void consume(TokenType type, const char* message) {
    if (parser.current.type == type) {
        advance();
        return;
    }

    errorAtCurrent(message);
}


// Compiles the raw source code string into bytecode instructions inside the chunk
bool compile(const char* source, Chunk* chunk) {
    initScanner(source);

    parser.hadError = false;
    parser.panicMode = false;

    advance();

    expression();
    consume(TOKEN_EOF, "Expect end of expression.");

    return !parser.hadError;
}