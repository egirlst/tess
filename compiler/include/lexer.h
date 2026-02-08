#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>

typedef enum {
    TOKEN_EOF,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_IDENTIFIER,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_SEMICOLON,
    TOKEN_ASSIGN,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_REPEAT,
    TOKEN_FOR,
    TOKEN_GT,
    TOKEN_LT,
    TOKEN_EQ,
    TOKEN_NEQ,
    TOKEN_GTE,
    TOKEN_LTE,
    TOKEN_COMMA,
    TOKEN_DOT,
    TOKEN_COLON,
    TOKEN_RETURN,
    TOKEN_BREAK,
    TOKEN_CONTINUE,
    TOKEN_CLASS,
    TOKEN_NEW,
    TOKEN_PRINT,
    TOKEN_REQUEST,
    TOKEN_HTTP,
    TOKEN_START,
    TOKEN_GET,
    TOKEN_ADD,
    TOKEN_GT_GT,
    TOKEN_LT_LT,
    TOKEN_EXCLAMATION,
    TOKEN_PERCENT,
    TOKEN_TRY,
    TOKEN_CATCH
} TessTokenType;

typedef struct {
    TessTokenType type;
    char *value;
    int line;
    int column;
} Token;

typedef struct {
    const char *source;
    size_t source_len;
    size_t position;
    int line;
    int column;
    Token *tokens;
    size_t token_count;
    size_t token_capacity;
} Lexer;

Lexer* lexer_create(const char *source);
void lexer_destroy(Lexer *lexer);
void lexer_tokenize(Lexer *lexer);

#endif
