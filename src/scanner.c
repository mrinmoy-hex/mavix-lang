#include <stdio.h>
#include <string.h>

#include "common.h"
#include "scanner.h"


// ============================================================================
// DATA STRUCTURES
// ============================================================================

/**
 * Scanner structure
 * Maintains the state needed to tokenize source code
 */
typedef struct {
    const char* start;          // Pointer to the start of the current lexeme
    const char* current;        // Pointer to the current character being examined
    int line;                   // Current line number (for error reporting)
} Scanner;


// ============================================================================
// GLOBAL STATE
// ============================================================================

Scanner scanner;                // Global scanner instance


// ============================================================================
// SCANNER INITIALIZATION
// ============================================================================

/**
 * Initializes the scanner with source code
 * Called once before tokenizing a complete source file
 * 
 * @param source  Pointer to the source code string to scan
 */
void initScanner(const char* source) {
    scanner.start = source;
    scanner.current = source;
    scanner.line = 1;
}


// ============================================================================
// CHARACTER CLASSIFICATION HELPERS
// ============================================================================

/**
 * Checks if a character is alphabetic or underscore
 * Used to identify identifiers and keywords
 * 
 * @param c  The character to test
 * @return  true if the character is a-z, A-Z, or _
 */
static bool isAlpha(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_';
}


/**
 * Checks if a character is a decimal digit
 * Used to identify numeric literals
 * 
 * @param c  The character to test
 * @return  true if the character is 0-9
 */
static bool isDigit(char c) {
    return c >= '0' && c <= '9';
}


// ============================================================================
// SCANNER POSITION & LOOKAHEAD
// ============================================================================

/**
 * Checks if the scanner has reached the end of the source
 * Tests for the null terminator
 * 
 * @return  true if at end of input
 */
static bool isAtEnd() {
    return *scanner.current == '\0';
}


/**
 * Consumes the next character and returns it
 * Advances the current pointer forward
 * 
 * @return  The character that was just consumed
 */
static char advance() {
    scanner.current++;
    return scanner.current[-1];
}


/**
 * Peeks at the current character without consuming it
 * Safe to call even at end of input (returns '\0')
 * 
 * @return  The current character without advancing
 */
static char peek() {
    return *scanner.current;
}


/**
 * Peeks at the next character without consuming current
 * Useful for two-character token lookahead
 * 
 * @return  The next character, or '\0' if at end
 */
static char peekNext() {
    if (isAtEnd()) {
        return '\0';
    }

    return scanner.current[1];
}


// ============================================================================
// TOKEN CREATION
// ============================================================================

/**
 * Creates a token of the given type using the current lexeme
 * The lexeme spans from scanner.start to scanner.current
 * 
 * @param type  The token type to assign
 * @return  A Token struct initialized with the current lexeme
 */
static Token makeToken(TokenType type) {
    Token token;
    token.type = type;
    token.start = scanner.start;
    token.length = (int)(scanner.current - scanner.start);
    token.line = scanner.line;
    return token;
}


/**
 * Creates an error token with an error message
 * Used to report scanning errors (e.g., unterminated string)
 * 
 * @param message  The error message to store in the token
 * @return  A Token of type TOKEN_ERROR with the message text
 */
static Token errorToken(const char* message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = scanner.line;
    return token;
}


// ============================================================================
// WHITESPACE & COMMENT HANDLING
// ============================================================================

/**
 * Skips whitespace and comments
 * Handles:
 *   - Space, tab, carriage return
 *   - Newlines (increments line counter)
 *   - Single-line comments (//)
 *   - Multi-line block comments
 * 
 * Continues until a non-whitespace, non-comment character is found
 */
static void skipWhitespace() {
    for (;;) {
        char c = peek();

        switch (c) {
            // Standard whitespace characters
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;

            // Newline: advance and increment line counter
            case '\n':
                scanner.line++;
                advance();
                break;

            // Comment handling
            case '/':
                if (peekNext() == '/') {
                    // Single-line comment: skip until end of line
                    while (peek() != '\n' && !isAtEnd()) {
                        advance();
                    }
                }
                else if (peekNext() == '*') {
                    // Multi-line block comment: /* ... */
                    advance();  // consume '/'

                    // Keep looking until we find '*/' or hit EOF
                    while (!isAtEnd()) {
                        // Track newlines inside the comment
                        if (peek() == '\n') {
                            scanner.line++;
                        }

                        // Found the end of the block comment
                        if (peek() == '*' && peekNext() == '/') {
                            advance();  // consume '*'
                            advance();  // consume '/'
                            break;
                        }

                        advance();  // continue consuming characters
                    }
                }
                else {
                    // Not a comment; '/' is an operator
                    return;
                }
                break;

            // Not whitespace or comment; stop skipping
            default:
                return;
        }
    }
}


// ============================================================================
// KEYWORD RECOGNITION
// ============================================================================

/**
 * Checks if an identifier matches a specific keyword
 * Performs a length check followed by byte-for-byte comparison
 * 
 * Implementation uses the Trie technique: checks the first letter separately
 * in identifierType(), then uses this to verify the rest of the keyword
 * 
 * @param start  Offset into the identifier to start comparing (after first letter)
 * @param length  The expected length of the remaining keyword
 * @param rest  The exact string of the remaining keyword to match
 * @param type  The token type to return if matched
 * @return  The token type if matched, TOKEN_IDENTIFIER otherwise
 */
static TokenType checkKeyword(int start, int length, const char* rest, TokenType type) {
    // Check if the total lexeme length matches the expected keyword length
    if (scanner.current - scanner.start == start + length) {
        // Compare the remaining characters byte-for-byte
        if (memcmp(scanner.start + start, rest, length) == 0) {
            return type;  // Exact match found
        }
    }

    // No match; this is just a regular identifier
    return TOKEN_IDENTIFIER;
}


/**
 * Determines the token type for an identifier or keyword
 * Uses a Trie-like technique: routes based on first character,
 * then second character for multi-option cases (f-words, t-words)
 * 
 * @return  TOKEN_KEYWORD type if this is a reserved word, TOKEN_IDENTIFIER otherwise
 */
static TokenType identifierType() {
    // Dispatch on the first character of the lexeme
    switch (scanner.start[0]) {
        case 'a':
            return checkKeyword(1, 2, "nd", TOKEN_AND);

        case 'c':
            return checkKeyword(1, 4, "lass", TOKEN_CLASS);

        case 'e':
            return checkKeyword(1, 3, "lse", TOKEN_ELSE);

        case 'i':
            return checkKeyword(1, 1, "f", TOKEN_IF);

        case 'n':
            return checkKeyword(1, 2, "il", TOKEN_NIL);

        case 'o':
            return checkKeyword(1, 1, "r", TOKEN_OR);

        case 'p':
            return checkKeyword(1, 4, "rint", TOKEN_PRINT);

        case 'r':
            return checkKeyword(1, 5, "eturn", TOKEN_RETURN);

        case 's':
            return checkKeyword(1, 4, "uper", TOKEN_SUPER);

        case 'v':
            return checkKeyword(1, 2, "ar", TOKEN_VAR);

        case 'w':
            return checkKeyword(1, 4, "hile", TOKEN_WHILE);

        // Handle keywords starting with 'f': false, for, fun
        case 'f':
            if (scanner.current - scanner.start > 1) {
                // Dispatch on the second character
                switch (scanner.start[1]) {
                    case 'a':
                        return checkKeyword(2, 3, "lse", TOKEN_FALSE);

                    case 'o':
                        return checkKeyword(2, 1, "r", TOKEN_FOR);

                    case 'u':
                        return checkKeyword(2, 1, "n", TOKEN_FUN);
                }
            }
            break;

        // Handle keywords starting with 't': this, true
        case 't':
            if (scanner.current - scanner.start > 1) {
                // Dispatch on the second character
                switch (scanner.start[1]) {
                    case 'h':
                        return checkKeyword(2, 2, "is", TOKEN_THIS);

                    case 'r':
                        return checkKeyword(2, 2, "ue", TOKEN_TRUE);
                }
            }
            break;
    }

    // Not a keyword; return as a regular identifier
    return TOKEN_IDENTIFIER;
}


// ============================================================================
// LITERAL SCANNING - IDENTIFIERS, NUMBERS, STRINGS
// ============================================================================

/**
 * Scans an identifier or keyword token
 * Continues while characters are alphabetic or numeric
 * Then checks if it's a reserved keyword
 * 
 * @return  A Token of type TOKEN_IDENTIFIER or a keyword type
 */
static Token identifier() {
    while (isAlpha(peek()) || isDigit(peek())) {
        advance();
    }

    return makeToken(identifierType());
}


/**
 * Scans a numeric literal token
 * Handles both integer and floating-point numbers
 * 
 * Format: [0-9]+ ('.' [0-9]+)?
 * Examples: 42, 3.14, 0, .5
 * 
 * @return  A Token of type TOKEN_NUMBER
 */
static Token number() {
    // Consume the integer part
    while (isDigit(peek())) {
        advance();
    }

    // Look for the fractional part
    if (peek() == '.' && isDigit(peekNext())) {
        // Consume the decimal point
        advance();

        // Consume the fractional digits
        while (isDigit(peek())) {
            advance();
        }
    }

    return makeToken(TOKEN_NUMBER);
}


/**
 * Scans a string literal token
 * Handles multi-line strings and tracks line numbers
 * 
 * Format: '"' (any character | escape sequence)* '"'
 * 
 * @return  A Token of type TOKEN_STRING, or an error token if unterminated
 */
static Token string() {
    // Keep consuming characters until we find the closing quote or EOF
    while (peek() != '"' && !isAtEnd()) {
        // Track line numbers in case the string spans multiple lines
        if (peek() == '\n') {
            scanner.line++;
        }

        advance();
    }

    // Check for unterminated string
    if (isAtEnd()) {
        return errorToken("Unterminated string.");
    }

    // Consume the closing double quote
    advance();
    return makeToken(TOKEN_STRING);
}


// ============================================================================
// TWO-CHARACTER TOKEN MATCHING
// ============================================================================

/**
 * Conditionally consumes the expected character
 * Used for two-character operators like ==, !=, <=, >=
 * 
 * @param expected  The character to match
 * @return  true if the current character matches and is consumed, false otherwise
 */
static bool match(char expected) {
    if (isAtEnd()) {
        return false;
    }

    if (*scanner.current != expected) {
        return false;
    }

    scanner.current++;
    return true;
}


// ============================================================================
// MAIN TOKENIZATION ENTRY POINT
// ============================================================================

/**
 * Scans the next token from the source code
 * This is the main public interface of the scanner
 * 
 * Algorithm:
 *   1. Skip whitespace and comments
 *   2. Mark the start of the new token
 *   3. Consume the first character and dispatch based on its type:
 *      - Alphabetic: identifier or keyword
 *      - Digit: numeric literal
 *      - Special character: operator or delimiter
 * 
 * @return  The next Token from the source, or TOKEN_EOF at end of input
 */
Token scanToken() {
    // Skip any leading whitespace and comments
    skipWhitespace();

    // Mark the start of the new token lexeme
    scanner.start = scanner.current;

    // Check for end of input
    if (isAtEnd()) {
        return makeToken(TOKEN_EOF);
    }

    // Get the first character of the token and advance past it
    char c = advance();

    // Dispatch based on the first character type
    if (isAlpha(c)) {
        return identifier();
    }

    if (isDigit(c)) {
        return number();
    }

    // Handle single and two-character operators
    switch (c) {
        // Single-character tokens
        case '(':
            return makeToken(TOKEN_LEFT_PAREN);

        case ')':
            return makeToken(TOKEN_RIGHT_PAREN);

        case '{':
            return makeToken(TOKEN_LEFT_BRACE);

        case '}':
            return makeToken(TOKEN_RIGHT_BRACE);

        case ';':
            return makeToken(TOKEN_SEMICOLON);

        case ',':
            return makeToken(TOKEN_COMMA);

        case '.':
            return makeToken(TOKEN_DOT);

        case '-':
            return makeToken(TOKEN_MINUS);

        case '+':
            return makeToken(TOKEN_PLUS);

        case '/':
            return makeToken(TOKEN_SLASH);

        case '*':
            return makeToken(TOKEN_STAR);

        // Two-character tokens: check for the second character
        case '!':
            return makeToken(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);

        case '=':
            return makeToken(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);

        case '<':
            return makeToken(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);

        case '>':
            return makeToken(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);

        // String literals
        case '"':
            return string();

        // Unrecognized character
        default:
            return errorToken("Unexpected character.");
    }
}