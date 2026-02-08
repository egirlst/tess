#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef enum {
    AST_PROGRAM,
    AST_NUMBER,
    AST_STRING,
    AST_IDENTIFIER,
    AST_BINARY_OP,
    AST_ASSIGNMENT,
    AST_FUNCTION_CALL,
    AST_BLOCK,
    AST_IF,
    AST_WHILE,
    AST_REPEAT,
    AST_RETURN,
    AST_FUNCTION_DEF,
    AST_CLASS_DEF,
    AST_NEW,
    AST_MEMBER_ACCESS,
    AST_LIST,
    AST_DICT,
    AST_INDEX,
    AST_PRINT,
    AST_HTTP_REQUEST,
    AST_MAIN_CALL,
    AST_IMPORT,
    AST_INNER_BLOCK,
    AST_BREAK,
    AST_CONTINUE,
    AST_VARIABLE_DECL,
    AST_TRY,
    AST_CATCH,
    AST_START
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    struct ASTNode *left;
    struct ASTNode *right;
    struct ASTNode *children;
    struct ASTNode *next;
    char *value;
    int line;
    int column;
} ASTNode;

typedef struct {
    Lexer *lexer;
    Token *tokens;
    size_t token_count;
    size_t token_index;
} Parser;

Parser* parser_create(Lexer *lexer);
void parser_destroy(Parser *parser);
ASTNode* parser_parse(Parser *parser);
ASTNode* ast_create_node(ASTNodeType type);
void ast_destroy_node(ASTNode *node);
void ast_destroy_tree(ASTNode *root);

#endif
