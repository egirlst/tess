#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

Lexer* lexer_create(const char *source) {
    Lexer *lexer = malloc(sizeof(Lexer));
    lexer->source = strdup(source);
    lexer->source_len = strlen(source);
    lexer->position = 0;
    lexer->line = 1;
    lexer->column = 1;
    lexer->tokens = NULL;
    lexer->token_count = 0;
    lexer->token_capacity = 0;
    return lexer;
}

void lexer_destroy(Lexer *lexer) {
    if (lexer) {
        free((void*)lexer->source);
        if (lexer->tokens) {
            for (size_t i = 0; i < lexer->token_count; i++) {
                if (lexer->tokens[i].value) {
                    free(lexer->tokens[i].value);
                }
            }
            free(lexer->tokens);
        }
        free(lexer);
    }
}

void lexer_skip_whitespace(Lexer *lexer) {
    while (lexer->position < lexer->source_len) {
        char c = lexer->source[lexer->position];
        if (c == ' ' || c == '\t' || c == '\r') {
            lexer->position++;
            lexer->column++;
        } else if (c == '\n') {
            lexer->position++;
            lexer->line++;
            lexer->column = 1;
        } else {
            break;
        }
    }
}

void lexer_skip_comment(Lexer *lexer) {
    if (lexer->position + 1 < lexer->source_len &&
        lexer->source[lexer->position] == '$' &&
        lexer->source[lexer->position + 1] == '$') {
        lexer->position += 2;
        lexer->column += 2;
        while (lexer->position < lexer->source_len) {
            if (lexer->source[lexer->position] == '\n') {
                lexer->line++;
                lexer->column = 1;
                lexer->position++;
                break;
            }
            lexer->position++;
            lexer->column++;
        }
        return;
    }
    
    if (lexer->position < lexer->source_len &&
        lexer->source[lexer->position] == '#') {
        lexer->position++;
        lexer->column++;
        while (lexer->position < lexer->source_len) {
            if (lexer->source[lexer->position] == '\n') {
                lexer->line++;
                lexer->column = 1;
                lexer->position++;
                break;
            }
            lexer->position++;
            lexer->column++;
        }
    }
}

Token lexer_next_token(Lexer *lexer) {
    while (lexer->position < lexer->source_len) {
        size_t old_pos = lexer->position;
        lexer_skip_whitespace(lexer);
        lexer_skip_comment(lexer);
        if (lexer->position == old_pos) {
            break;
        }
    }

    if (lexer->position >= lexer->source_len) {
        Token token = {TOKEN_EOF, NULL, lexer->line, lexer->column};
        return token;
    }

    char c = lexer->source[lexer->position];
    int start_line = lexer->line;
    int start_col = lexer->column;

    Token token;
    token.line = start_line;
    token.column = start_col;
    token.value = NULL;

    if (c == '<') {
        if (lexer->position + 1 < lexer->source_len && lexer->source[lexer->position + 1] == '<') {
            token.type = TOKEN_LT_LT;
            token.value = strdup("<<");
            lexer->position += 2; lexer->column += 2;
            return token;
        }
        if (lexer->position + 1 < lexer->source_len && lexer->source[lexer->position + 1] == '=') {
            token.type = TOKEN_LTE;
            token.value = strdup("<=");
            lexer->position += 2; lexer->column += 2;
            return token;
        }
        token.type = TOKEN_LT;
        token.value = strdup("<");
        lexer->position++; lexer->column++;
        return token;
    }

    if (c == '>') {
        if (lexer->position + 1 < lexer->source_len && lexer->source[lexer->position + 1] == '>') {
            token.type = TOKEN_GT_GT;
            token.value = strdup(">>");
            lexer->position += 2; lexer->column += 2;
            return token;
        }
        if (lexer->position + 1 < lexer->source_len && lexer->source[lexer->position + 1] == '=') {
            token.type = TOKEN_GTE;
            token.value = strdup(">=");
            lexer->position += 2; lexer->column += 2;
            return token;
        }
        token.type = TOKEN_GT;
        token.value = strdup(">");
        lexer->position++; lexer->column++;
        return token;
    }

    if (c == '=') {
        if (lexer->position + 1 < lexer->source_len && lexer->source[lexer->position + 1] == '=') {
            token.type = TOKEN_EQ;
            token.value = strdup("==");
            lexer->position += 2; lexer->column += 2;
            return token;
        }
        token.type = TOKEN_ASSIGN;
        token.value = strdup("=");
        lexer->position++; lexer->column++;
        return token;
    }
    
    if (c == '!') {
        if (lexer->position + 1 < lexer->source_len && lexer->source[lexer->position + 1] == '=') {
            token.type = TOKEN_NEQ;
            token.value = strdup("!=");
            lexer->position += 2; lexer->column += 2;
            return token;
        }
        token.type = TOKEN_EXCLAMATION;
        token.value = strdup("!");
        lexer->position++; lexer->column++;
        return token;
    }

    switch (c) {
        case '+': 
            token.type = TOKEN_PLUS; token.value = strdup("+"); 
            lexer->position++; lexer->column++; return token;
        case '-': 
            token.type = TOKEN_MINUS; token.value = strdup("-"); 
            lexer->position++; lexer->column++; return token;
        case '*': 
            token.type = TOKEN_MULTIPLY; token.value = strdup("*"); 
            lexer->position++; lexer->column++; return token;
        case '/': 
            token.type = TOKEN_DIVIDE; token.value = strdup("/"); 
            lexer->position++; lexer->column++; return token;
        case '%': 
            token.type = TOKEN_PERCENT; token.value = strdup("%"); 
            lexer->position++; lexer->column++; return token;
        case '(': 
            token.type = TOKEN_LPAREN; token.value = strdup("("); 
            lexer->position++; lexer->column++; return token;
        case ')': 
            token.type = TOKEN_RPAREN; token.value = strdup(")"); 
            lexer->position++; lexer->column++; return token;
        case '{': 
            token.type = TOKEN_LBRACE; token.value = strdup("{"); 
            lexer->position++; lexer->column++; return token;
        case '}': 
            token.type = TOKEN_RBRACE; token.value = strdup("}"); 
            lexer->position++; lexer->column++; return token;
        case '[': 
            token.type = TOKEN_LBRACKET; token.value = strdup("["); 
            lexer->position++; lexer->column++; return token;
        case ']': 
            token.type = TOKEN_RBRACKET; token.value = strdup("]"); 
            lexer->position++; lexer->column++; return token;
        case ':': 
            if (lexer->position + 1 < lexer->source_len && lexer->source[lexer->position + 1] == ':') {
                token.type = TOKEN_COLON; token.value = strdup(":"); 
                lexer->position++; lexer->column++; return token;
            }
            token.type = TOKEN_COLON; token.value = strdup(":"); 
            lexer->position++; lexer->column++; return token;
        case ';': 
            token.type = TOKEN_SEMICOLON; token.value = strdup(";"); 
            lexer->position++; lexer->column++; return token;
        case ',': 
            token.type = TOKEN_COMMA; token.value = strdup(","); 
            lexer->position++; lexer->column++; return token;
        case '.': 
            token.type = TOKEN_DOT; token.value = strdup("."); 
            lexer->position++; lexer->column++; return token;
    }

    if (c == '"') {
        lexer->position++;
        lexer->column++;
        size_t str_len = 0;
        char *str = malloc(1024);
        if (!str) {
            token.type = TOKEN_EOF;
            return token;
        }
        
        while (lexer->position < lexer->source_len) {
            if (lexer->source[lexer->position] == '\\' && 
                lexer->position + 1 < lexer->source_len) {
                lexer->position++;
                char esc = lexer->source[lexer->position];
                switch (esc) {
                    case 'n': str[str_len++] = '\n'; break;
                    case 't': str[str_len++] = '\t'; break;
                    case 'r': str[str_len++] = '\r'; break;
                    case '\\': str[str_len++] = '\\'; break;
                    case '"': str[str_len++] = '"'; break;
                    default: str[str_len++] = esc; break;
                }
                lexer->position++;
                lexer->column += 2;
            } else if (lexer->source[lexer->position] == '"') {
                lexer->position++;
                lexer->column++;
                break;
            } else {
                if (lexer->source[lexer->position] == '\n') {
                    lexer->line++;
                    lexer->column = 1;
                } else {
                    lexer->column++;
                }
                str[str_len++] = lexer->source[lexer->position++];
            }
            
            if (str_len >= 1023) {
                char *new_str = realloc(str, str_len + 1024);
                if (!new_str) break;
                str = new_str;
            }
        }
        
        str[str_len] = '\0';
        token.type = TOKEN_STRING;
        token.value = str;
        return token;
    }

    if (isdigit(c)) {
        size_t start = lexer->position;
        while (lexer->position < lexer->source_len && 
               (isdigit(lexer->source[lexer->position]) || 
                lexer->source[lexer->position] == '.')) {
            lexer->position++;
            lexer->column++;
        }
        size_t len = lexer->position - start;
        char *num = malloc(len + 1);
        memcpy(num, lexer->source + start, len);
        num[len] = '\0';
        token.type = TOKEN_NUMBER;
        token.value = num;
        return token;
    }

    if (isalpha(c) || c == '_') {
        size_t start = lexer->position;
        while (lexer->position < lexer->source_len && 
               (isalnum(lexer->source[lexer->position]) || 
                lexer->source[lexer->position] == '_' ||
                lexer->source[lexer->position] == ':')) {
            lexer->position++;
            lexer->column++;
        }
        size_t len = lexer->position - start;
        char *ident = malloc(len + 1);
        memcpy(ident, lexer->source + start, len);
        ident[len] = '\0';

        token.value = ident;
        token.type = TOKEN_IDENTIFIER;

        if (strcmp(ident, "if") == 0) token.type = TOKEN_IF;
        else if (strcmp(ident, "else") == 0) token.type = TOKEN_ELSE;
        else if (strcmp(ident, "while") == 0) token.type = TOKEN_WHILE;
        else if (strcmp(ident, "repeat") == 0) token.type = TOKEN_REPEAT;
        else if (strcmp(ident, "for") == 0) token.type = TOKEN_FOR;
        else if (strcmp(ident, "break") == 0) token.type = TOKEN_BREAK;
        else if (strcmp(ident, "continue") == 0) token.type = TOKEN_CONTINUE;
        else if (strcmp(ident, "ret") == 0) token.type = TOKEN_RETURN;
        else if (strcmp(ident, "cls") == 0) token.type = TOKEN_CLASS;
        else if (strcmp(ident, "new") == 0) token.type = TOKEN_NEW;
        else if (strcmp(ident, "try") == 0) token.type = TOKEN_TRY;
        else if (strcmp(ident, "catch") == 0) token.type = TOKEN_CATCH;
        else if (strcmp(ident, "print::") == 0) token.type = TOKEN_PRINT;
        else if (strcmp(ident, "request::") == 0) token.type = TOKEN_REQUEST;
        else if (strcmp(ident, "http::") == 0) token.type = TOKEN_HTTP;
        else if (strcmp(ident, "start") == 0) token.type = TOKEN_START;
        else if (strcmp(ident, "get") == 0) token.type = TOKEN_GET;
        else if (strcmp(ident, "add") == 0) token.type = TOKEN_ADD;

        return token;
    }

    lexer->position++;
    lexer->column++;
    token.type = TOKEN_EOF;
    return token;
}

void lexer_tokenize(Lexer *lexer) {
    lexer->token_count = 0;
    lexer->token_capacity = 16;
    lexer->tokens = malloc(sizeof(Token) * lexer->token_capacity);
    
    while (1) {
        Token token = lexer_next_token(lexer);
        
        if (lexer->token_count >= lexer->token_capacity) {
            lexer->token_capacity *= 2;
            lexer->tokens = realloc(lexer->tokens, sizeof(Token) * lexer->token_capacity);
        }
        
        lexer->tokens[lexer->token_count++] = token;
        
        if (token.type == TOKEN_EOF) break;
    }
}
