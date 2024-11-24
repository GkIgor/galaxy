#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../../include/lexer/core.h"
#include "../../../include/lexer/error.h"
#include "../../../include/utils.h"

Token *tokens = NULL;
int tokenCount = 0;
FILE *src = NULL;
char currentChar;
int line = 1;
int col = 1;
int position = 0;
const char *filename = NULL;

/**
 * @brief Initializes the lexer with a source file and filename.
 * 
 * Sets up the lexer state, including the source file, filename, and the initial character.
 * 
 * @param source The source file to tokenize.
 * @param file The name of the source file.
 */
void initLexer(FILE *source, const char *file) {
    src = source;
    filename = file;
    currentChar = fgetc(src);
}

/**
 * @brief Skips whitespace characters in the source file.
 * 
 * Advances the lexer past spaces, tabs, and newlines, updating line, column, and position counters.
 */
void skipWhitespace() {
    while (isspace(currentChar)) {
        if (currentChar == '\n') {
            line++;
            col = 1;
        } else {
            col++;
        }
        currentChar = fgetc(src);
        position++;
    }
}

/**
 * @brief Safely duplicates a string.
 * 
 * Creates a copy of the given string, ensuring memory allocation errors are handled.
 * 
 * @param str The string to duplicate. If NULL, an empty string is returned.
 * @return A pointer to the duplicated string.
 */
char *safe_strdup(const char *str) {
    if (!str) return strdup("");
    char *dup = strdup(str);
    if (!dup) {
        fprintf(stderr, "Error duplicating string\n");
        exit(EXIT_FAILURE);
    }
    return dup;
}

/**
 * @brief Adds a new token to the token array.
 * 
 * Dynamically resizes the token array and appends a new token with the provided details.
 * 
 * @param type The type of the token.
 * @param lexeme The token's text.
 * @param line The line number where the token starts.
 * @param col_s The starting column of the token.
 * @param col_e The ending column of the token.
 * @param pos_s The starting position in the source file.
 * @param pos_e The ending position in the source file.
 * @param filename The name of the source file.
 * @param message An optional message associated with the token.
 */
void addToken(TokenType type, const char *lexeme, int line, int col_s, int col_e, int pos_s, int pos_e, const char *filename, char *message) {
    tokens = realloc(tokens, sizeof(Token) * (tokenCount + 1));
    Token *newToken = &tokens[tokenCount];
    newToken->type = type;
    newToken->lexeme = safe_strdup(lexeme);
    newToken->line = line;
    newToken->column_start = col_s;
    newToken->column_end = col_e;
    newToken->position_start = pos_s;
    newToken->position_end = pos_e;
    newToken->filename = filename;
    newToken->message = safe_strdup(message);
    tokenCount++;
}

/**
 * @brief Consumes and returns the current character from the source file.
 * 
 * Advances the lexer to the next character, updating column and position counters.
 * 
 * @return The consumed character.
 */
char eat_char() {
    char c = currentChar;
    currentChar = fgetc(src);
    col++;
    position++;
    return c;
}

/**
 * @brief Returns the current character without consuming it.
 * 
 * @return The current character in the source file.
 */
char pick_char() {
    return currentChar;
}

/**
 * @brief Peeks at the next character without consuming it.
 * 
 * Reads the next character and restores the source file's position.
 * 
 * @return The next character in the source file.
 */
char pick_next() {
    char next = fgetc(src);
    ungetc(next, src);
    return next;
}

/**
 * @brief Matches a keyword to its corresponding token type.
 * 
 * Checks if the given lexeme is a reserved keyword or an identifier.
 * 
 * @param lexeme The string to match.
 * @return The corresponding token type.
 */
TokenType match_keyword(const char *lexeme) {
    if (strcmp(lexeme, "for") == 0) return TOKEN_FOR;
    if (strcmp(lexeme, "if") == 0) return TOKEN_IF;
    if (strcmp(lexeme, "elif") == 0) return TOKEN_ELIF;
    if (strcmp(lexeme, "else") == 0) return TOKEN_ELSE;
    if (strcmp(lexeme, "switch") == 0) return TOKEN_SWITCH;
    if (strcmp(lexeme, "case") == 0) return TOKEN_CASE;
    if (strcmp(lexeme, "default") == 0) return TOKEN_DEFAULT;
    if (strcmp(lexeme, "package") == 0) return TOKEN_PACKAGE;
    if (strcmp(lexeme, "import") == 0) return TOKEN_IMPORT;
    if (strcmp(lexeme, "def") == 0) return TOKEN_DEF;
    if (strcmp(lexeme, "return") == 0) return TOKEN_RETURN;
    if (strcmp(lexeme, "end") == 0) return TOKEN_END;
    if (strcmp(lexeme, "true") == 0) return TOKEN_TRUE;
    if (strcmp(lexeme, "false") == 0) return TOKEN_FALSE;
    return TOKEN_IDENTIFIER;
}

/**
 * @brief Matches a single-character operator to its token type.
 * 
 * @param op The operator character.
 * @return The corresponding token type, or TOKEN_UNKNOWN if no match is found.
 */
TokenType match_operator(char op) {
    switch (op) {
        case '+': return TOKEN_PLUS;
        case '-': return TOKEN_MINUS;
        case '*': return TOKEN_MUL;
        case '/': return TOKEN_DIV;
        case '%': return TOKEN_MODULUS;
        case '<': return TOKEN_LT;
        case '>': return TOKEN_GT;
        case '^': return TOKEN_CARET;
        case '.': return TOKEN_DOT;
        case ':': return TOKEN_COLON;
        case ',': return TOKEN_COMMA;
        case ';': return TOKEN_SEMICOLON;
        case '(': return TOKEN_OPAREN;
        case ')': return TOKEN_CPAREN;
        default: return TOKEN_UNKNOWN;
    }
}

/**
 * @brief Matches a two-character operator to its token type.
 * 
 * @param first The first character of the operator.
 * @param second The second character of the operator.
 * @return The corresponding token type, or TOKEN_UNKNOWN if no match is found.
 */
TokenType match_two_char_operators(char first, char second) {
    if (first == '*' && second == '*') return TOKEN_POWER;
    if (first == '-' && second == '>') return TOKEN_ARROW;
    if (first == '.' && second == '.') return TOKEN_RANGE;
    if (first == '<' && second == '=') return TOKEN_LEQUAL;
    if (first == '>' && second == '=') return TOKEN_GEQUAL;
    if (first == '=' && second == '=') return TOKEN_EQUAL;
    if (first == ':' && second == '=') return TOKEN_ASSIGN;
    return TOKEN_UNKNOWN;
}

/**
 * @brief Retrieves the next token from the source file.
 * 
 * Processes the source file to identify and return the next valid token. Handles keywords, 
 * operators, numbers, strings, and errors.
 * 
 * @return The next token.
 */
Token getNextToken() {
    skipWhitespace();

    // Identifiers and keywords
    if (isalpha(pick_char()) || pick_char() == '_') {
        char buffer[256];
        int i = 0;
        while (isalnum(pick_char()) || pick_char() == '_') {
            if (i >= (int)sizeof(buffer) - 1) {
                lexer_error(filename, line, col, position, position, currentChar, "Identifier too long");
                break;
            }
            buffer[i++] = eat_char();
        }
        buffer[i] = '\0';
        TokenType type = match_keyword(buffer);
        return (Token){type, safe_strdup(buffer), line, col - i, col, position - i, position, filename, ""};
    }

    // Numbers (integer and decimal)
    if (isdigit(pick_char())) {
        char buffer[256];
        int i = 0;
        int isDecimal = 0;
        while (isdigit(pick_char()) || (!isDecimal && pick_char() == '.')) {
            if (pick_char() == '.') {
                isDecimal = 1;
            }
            buffer[i++] = eat_char();
        }
        buffer[i] = '\0';
        return (Token){
            TOKEN_NUMBER,
            safe_strdup(buffer),
            line, col - i, col, position - i, position, filename, ""
        };
    }

    // Strings
    if (pick_char() == '"') {
        char buffer[1024];
        int i = 0;
        eat_char(); // Consume the opening quote
        while (pick_char() != '"' && pick_char() != EOF) {
            if (i >= (int)sizeof(buffer) - 1) {
                lexer_error(filename, line, col, position, position, currentChar, "String too long");
                break;
            }
            buffer[i++] = eat_char();
        }
        if (pick_char() == '"') {
            eat_char(); // Consume the closing quote
        } else {
            lexer_error(filename, line, col, position, position, currentChar, "Unterminated string");
        }
        buffer[i] = '\0';
        return (Token){TOKEN_STRING, safe_strdup(buffer), line, col - i - 1, col, position - i - 1, position, filename, ""};
    }

    // Two-character operators
    char current = pick_char();
    char next = pick_next();
    TokenType twoCharType = match_two_char_operators(current, next);
    if (twoCharType != TOKEN_UNKNOWN) {
        eat_char(); // Consume first char
        eat_char(); // Consume second char
        char buffer[3] = {current, next, '\0'};
        return (Token){twoCharType, safe_strdup(buffer), line, col - 2, col, position - 2, position, filename, ""};
    }

    // Single-character operators
    if (strchr("+-*/%<>^=.,:;(){}", pick_char())) {
        char op = eat_char();
        TokenType type = match_operator(op);
        if (type != TOKEN_UNKNOWN) {
            char buffer[2] = {op, '\0'};
            return (Token){type, safe_strdup(buffer), line, col - 1, col, position - 1, position, filename, ""};
        }
    }

    // EOF
    if (pick_char() == EOF) {
        return (Token){TOKEN_EOF, safe_strdup("EOF"), line, col, col, position, position, filename, ""};
    }

    // Unknown character
    lexer_error(filename, line, col, position - 1, position, currentChar, "Invalid character");
    eat_char();
    return (Token){TOKEN_UNKNOWN, safe_strdup(""), line, col - 1, col - 1, position - 1, position - 1, filename, ""};
}

/**
 * @brief Tokenizes the contents of a source file.
 * 
 * Processes the entire source file, generating an array of tokens and storing the token count.
 * 
 * @param sourceFile The source file to tokenize.
 * @param fileName The name of the source file.
 * @param count Pointer to store the total token count.
 * @return A dynamically allocated array of tokens.
 */
Token *tokenize(FILE *sourceFile, const char *fileName, int *count) {
    initLexer(sourceFile, fileName);
    tokens = NULL;
    tokenCount = 0;
    Token token = getNextToken();

    while (currentChar != EOF) {
        addToken(
            token.type,
            token.lexeme,
            token.line,
            token.column_start,
            token.column_end,
            token.position_start,
            token.position_end,
            token.filename,
            token.message
        );
        token = getNextToken();
    }

    addToken(
        token.type,
        token.lexeme,
        token.line,
        token.column_start,
        token.column_end,
        token.position_start,
        token.position_end,
        token.filename,
        token.message
    );

    int hasEOF = 0;
    for (int i = 0; i < tokenCount; i++) {
        if (tokens[i].type == TOKEN_EOF) {
            hasEOF = 1;
            break;
        }
    }

    if (!hasEOF) {
        addToken(
            TOKEN_EOF,
            "EOF",
            line,
            col,
            col,
            position,
            position,
            filename,
            ""
        );
    }

    *count = tokenCount;
    return tokens;
}
