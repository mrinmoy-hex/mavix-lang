//
// Created by mrinmoy on 6/7/25.
//

#include <stdio.h>
#include <string.h>

#include "common.h"
#include "scanner.h"

typedef struct {
    const char* start;      // beginning of current lexeme
    const char* current;    // points to current char being looked at
    int line;               // 
} Scanner;

Scanner scanner;        // Global instance of the scanner

// Initializes the scanner with the source code
void initScanner(const char* source) {
    scanner.start = source;
    scanner.current = source;
    scanner.line = 1;           // Line number starts at 1
}

// Returns true if we've reached the end of the source string
static bool isAtEnd() {
    return *scanner.current == '\0';
}


static char advance() {
    scanner.current++;              // Move to the next character
    return scanner.current[-1];     // Return the character we just passed
}

// Look at the current characer without consuming it
static char peek() {
    return *scanner.current;
}

static bool match(char expected_char) {
    if (isAtEnd()) return false;
    if (*scanner.current != expected_char) return false;
    scanner.current++;      // Adances the pointer if matches
    return true;
}


// Creates and returns a valid token
static Token makeToken(TokenType type) {
    Token token;
    token.type = type;
    token.start = scanner.start;        // Start o fthe lexeme
    token.length = (int) (scanner.current - scanner.start);     // lexeme length
    token.line = scanner.line;
    return token;
}

// Creates and returns an error token with a message
static Token errorToken(const char* message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;          // Points to static error message
    token.length = (int) strlen(message);
    token.line = scanner.line;
    return token;
}


static void skipWhitespace() {
    for (;;) {
        char c = peek();
        switch (c) {
            // For space, carriage return, and tab,
            // we consume the character and continue skipping
            case ' ':       // space
            case '\r':      // carriage return 
            case '\t':      // tab
                advance();
                break;
            // handles new line
            case '\n':
                scanner.line++;
                advance();
                break;
            default:
                return;
        }
    }
} 


// Scans the next token from the input and returns it
Token scanToken() {
    skipWhitespace();
    scanner.start = scanner.current;    // Mark the start of the next lexeme

    // Returns EOF, if we've reached the end of the input
    if (isAtEnd())  return makeToken(TOKEN_EOF);


    char c = advance();

    switch (c) {
        // Scanning single-character token
        case '(': return makeToken(TOKEN_LEFT_PAREN);
        case ')': return makeToken(TOKEN_RIGHT_PAREN);
        case '{': return makeToken(TOKEN_LEFT_BRACE);
        case '}': return makeToken(TOKEN_RIGHT_BRACE);
        case ';': return makeToken(TOKEN_SEMICOLON);
        case ',': return makeToken(TOKEN_COMMA);
        case '.': return makeToken(TOKEN_DOT);
        case '-': return makeToken(TOKEN_MINUS);
        case '+': return makeToken(TOKEN_PLUS);
        case '/': return makeToken(TOKEN_SLASH);
        case '*': return makeToken(TOKEN_STAR);

        // Handling two-character tokens
        case '!':
            return makeToken(
                match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
        case '=':
            return makeToken(
                match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL
            );
        case '<':
            return makeToken(
                match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS
            );
        case '>':
            return makeToken(
                match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER
            );
    }


    // If not known matched, return an error token
    return errorToken("Unexpected character.");
}
