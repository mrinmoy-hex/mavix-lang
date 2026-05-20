#include <stdio.h>
#include <string.h>

#include "common.h"
#include "scanner.h"    


typedef struct {
    const char* start;      // start of the current lexeme
    const char* current;
    int line;               // for error report
} Scanner;

Scanner scanner;

void initScanner(const char* source) {
    scanner.start = source;
    scanner.current = source;
    scanner.line = 1;
}

// check if the scanner has reached the null terminator of the source 
static bool isAtEnd() {
    return *scanner.current == '\0';
}

// create a token with the given type and the current lexeme
static Token makeToken(TokenType type) {
    Token token;
    token.type = type;
    token.start = scanner.start;
    token.length = (int) (scanner.current - scanner.start);
    token.line = scanner.line;
    return token;
}


static Token errorToken(const char* message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = scanner.line;
    return token;
}

// look at the current character without consuming it
static char peek() {
    return *scanner.current;
}

// look at the next character without consuming it
static char peekNext() {
    if (isAtEnd()) return '\0';
    return scanner.current[1];
}

static void skipWhitespace() {
    for (;;) {
        char c = peek();
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;
            // 
            case '\n':
                scanner.line++;
                advance();
                break;
            // for comments
            case '/':
                if (peekNext() == '/') {
                    // a comment goes until the eol
                    while (peek() != '\n' && !isAtEnd()) advance();
                } 
                else if (peek() == '*') {
                    // multi-line block comment: /*
                    advance();

                    // keep looking until we find '*/' or hit the end of file
                    while (!isAtEnd()) {
                        if (peek() == '*' && peekNext() == '/') {
                            // consume both chars
                            advance();
                            advance();
                            break;
                        }

                        if (peek() == '\n') {
                            scanner.line++;
                        }

                        advance();      // keep eating up chars inside the comment
                    }
                } else {
                    return;
                }
                break;
            default:
                return;
        }
    }
}


static Token string() {
    // keep consuming until we find the closing quote or hit the end of file
    while (peek() != '"' && !isAtEnd()) {
        // keep track of line numbers in case the string literal spans multiple lines (multiline string)
        if (peek() == '\n') scanner.line++;
        advance();
    }

    if (isAtEnd())  return errorToken("Unterminated string.");

    // The closing quote
    advance();
    return makeToken(TOKEN_STRING);
}

// consume the next character and return it
static char advance() {
    scanner.current++;
    return scanner.current[-1];
}

// if the next character matches the expected one, consume it and return true
static bool match(char expected) {
    if (isAtEnd())  return false;
    if (*scanner.current != expected) return false;
    scanner.current++;
    return true;
}

// scans the next token from the source and returns it
Token scanToken() {
    skipWhitespace();
    // reset start pointer to the beginning of the new token
    scanner.start = scanner.current;

    if (isAtEnd())  return makeToken(TOKEN_EOF);

    char c = advance();

    switch (c) {
        // single char
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
        // two char tokens
        case: '!':
            return makeToken(
                match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG
            );
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

        // scanning literal tokens
        case '"': return string();

    }

    // for characters we haven't implemented parsing for yet
    return errorToken("Unexpected character.");
}