//
// Created by mrinmoy on 6/7/25.
//

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


// Reports a syntax error at the given token with the provided message
static void errorAt(Token* token, const char* message) {
    // If we're already in panic mode, don't report more errors.
    // This avoids flooding the user with cascading errors.
    if (parser.panicMode) return;

    // Enters panic mode to suppress further error messages 
    // until the parser has recovered (via synchronize()).
    parser.panicMode = true;

    // Print the error message with line number
    fprintf(stderr, "[line %d] Error", token->line);

    if (token->type == TOKEN_EOF) {
        // If at end of file, indicate that.
        fprintf(stderr, "at end");
    } else if (token->type == TOKEN_ERROR) {
        // If it's already an error token (like from the scanner),
        // don't print any more context.
        // The message itself already contains the error detail.
    } else {
        // Otherwise, print the exact token where the error occurred.
        fprintf(stderr, "at '%.*s'", token->length, token->start);
    }

    // Set a flag indicating that an error has occurred.
    // This is used to prevent code generation or running if there were syntax issues.
    fprintf(stderr, ": %s\n", message);
    parser.hadError = true;
}

// Reports an error at the previous token (typically used when a rule fails after consuming a token).
static void error(const char* message) {
    errorAt(&parser.previous, message);
}

// Reports an error at the current token (typically used when we detect an error before consuming a token).
static void errorAtCurrent(const char* message) {
    errorAt(&parser.current, message);
}

// Advances the parser to the next token.
// Skips over any error tokens produced by the scanner.
// If a token is an error, report it and keep scanning until a valid token is found.
static void advance() {
    parser.previous = parser.current;

    for (;;) {
        parser.current = scanToken();

        // Break the loop if we successfully scanned a valid token
        if (parser.current.type != TOKEN_ERROR) break;

        // Report error for invalid token and continue scanning.
        // The start of the token itself is reused as the error message in this case.
        errorAtCurrent(parser.current.start);
    }
}

// Consumes the current token if it matches the expected type.
// If it doesn't match, report an error at the current token.
static void consume(TokenType type, const char* message) {

    // If the token matches the expected type, advance to the next token.
    if (parser.current.type == type) {
        advance();
        return;
    }

    // Otherwise, report an error at the unexpected token.
    // This will also trigger panic mode to avoid further errors.
    errorAtCurrent(message);
}



/*
Working mechanism of compiler (Mavix)
- The compiler will take the user’s program and fill up the chunk with bytecode.
Then it sends the completed chunk over to the VM to be executed. 
*/
bool compile(const char* source, Chunk* chunk) {
    initScanner(source);

    parser.hadError = false;
    parser.panicMode = false;

    advance();
    expression();
    consume(TOKEN_EOF, "Expect end of expression.");

    return !parser.hadError;
}
