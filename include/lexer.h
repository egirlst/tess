#ifndef LEXER_H
#define LEXER_H

#include <stdlib.h>

typedef enum {
    TOKEN_EOF,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_PERCENT,  /* % */
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_ASSIGN,   /* = */
    TOKEN_EQ,       /* == */
    TOKEN_NEQ,      /* != */
    TOKEN_SEMICOLON,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_REPEAT,
    TOKEN_GT,
    TOKEN_LT,
    TOKEN_GTE,      /* >= */
    TOKEN_LTE,      /* <= */
    TOKEN_LT_LT,    /* << */
    TOKEN_GT_GT,    /* >> */
    TOKEN_EXCLAMATION, /* ! */
    TOKEN_PRINT,    /* print:: */
    TOKEN_DOT,      /* . */
    TOKEN_COMMA,    /* , */
    TOKEN_CLASS,    /* cls */
    TOKEN_RETURN,   /* ret */
    TOKEN_REQUEST,  /* request:: */
    TOKEN_HTTP,     /* http:: */
    TOKEN_WHILE,
    TOKEN_FOR,
    TOKEN_BREAK,
    TOKEN_CONTINUE,
    TOKEN_TRY,
    TOKEN_CATCH,
    TOKEN_NEW,
    TOKEN_LBRACKET, /* [ */
    TOKEN_RBRACKET, /* ] */
    TOKEN_COLON,    /* : */
    TOKEN_START,    /* start */
    TOKEN_GET,      /* get (deprecated, use add) */
    TOKEN_ADD       /* add */
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
Token lexer_next_token(Lexer *lexer);
Token lexer_peek_token(Lexer *lexer, int offset);

#endif /* LEXER_H */
