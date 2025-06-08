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

static bool isDigit(char c) {
    return c >= '0' && c <= '9';
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

static char peekNext() {
    if (isAtEnd())  return '\0';
    return scanner.current[1];      // looks one char ahead
}

static bool match(char expected_char) {
    if (isAtEnd()) return false;
    if (*scanner.current != expected_char) return false;
    scanner.current++;      // Adances the pointer if matches
    return true;
}


/**
 * @brief Creates a new token of the specified type.
 *
 * This function initializes a new token with the given type.
 *
 * @param type The type of the token to be created.
 * @return A new token of the specified type.
 */
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


/**
 * @brief Skips over any whitespace characters in the input.
 *
 * This function advances the input pointer past any whitespace characters
 * (such as spaces, tabs, and newlines) until it encounters a non-whitespace
 * character or the end of the input.
 */
static void skipWhitespace() {
    for (;;) {
        char c = peek();
        switch (c) {
            case ' ':
            // also checks for carriage returns
            case '\r':
            case '\t':
                advance();
                break;

            // handle newlines
            case '\n':
                scanner.line++;
                advance();
                break;

            // handle single-line comments
            case '/':
                // single-line comments
                if (peekNext() == '/') {
                    // A comment goes until the end of the line
                    while (peek() != '\n' && !isAtEnd()) advance();
                }
                // multi-line comments 
                else if (peek() == '*') {
                    advance();      // consume '*'
                    advance();      // move past the '/'

                    while (!isAtEnd()) {    
                        if (peek() == '\n') scanner.line++;     // trace newlines

                        if (peek() == '*' && peekNext() == '/') {
                            advance();      // consume '*'
                            advance();      // consume '/'
                            break;          // exit the loop after finding '*/'
                        }
                        
                        advance();          // continue scanning inside the comment
                    }
                    
                    // unterminated comment error
                    if (isAtEnd()) {
                        // printf("Error: Unterminated multi-line comment error");
                        errorToken("Unterminated multiline comment error.");
                    }
                } else {
                        return;     // not a comment, return
                    }
                
                break;
            default:
                return;
        }
    }
}

/** 
  *  @note This scanner does not convert the value immediately. It only stores the raw text(lexeme)
  *  as it appears in the source code
**/

static Token number() {

    while (isDigit(peek())) advance();

    // Look for a fractional part
    if (peek() == '.' && isDigit(peekNext())) {
        // Consume the '.'
        advance();

        while (isDigit(peek())) advance();
    }

    return makeToken(TOKEN_NUMBER);
}



static Token string() {
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') scanner.line++;
        advance();
    }

    if (isAtEnd())  return errorToken("Unterminated string.");

    // The closing quote
    advance();
    return makeToken(TOKEN_STRING);
}



/**
 * @brief Scans and returns the next token from the input source.
 *
 * This function reads characters from the input source and constructs
 * the next token to be processed by the lexer. It handles various types
 * of tokens including keywords, identifiers, literals, and operators.
 *
 * @return Token The next token from the input source.
 */
Token scanToken() {
    /**
     * @brief we are at the beginning of a new token when we enter the function. 
     * Thus, we set scanner.start to point to the current character so we remember where the 
     * lexeme we’re about to scan starts.
     */
    skipWhitespace();
    scanner.start = scanner.current;    // Mark the start of the next lexeme

    // Returns EOF, if we've reached the end of the input
    if (isAtEnd())  return makeToken(TOKEN_EOF);

    // We then consume the current character and return a token for it.
    char c = advance();

    if (isDigit(c)) return number();

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
        // Scanning literals
        case '"': return string();
    }


    // If not known matched, return an error token
    return errorToken("Unexpected character.");
}
