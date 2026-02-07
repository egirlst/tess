#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef enum {
    AST_PROGRAM,
    AST_IMPORT,
    AST_VARIABLE_DECL,
    AST_FUNCTION_DEF,
    AST_CLASS_DEF,
    AST_FUNCTION_CALL,    /* >name< */
    AST_MAIN_CALL,        /* >.< */
    AST_START,            /* start >.< */
    AST_REPEAT,
    AST_WHILE,
    AST_FOR,
    AST_IF,
    AST_ELSE,
    AST_BREAK,
    AST_CONTINUE,
    AST_RETURN,
    AST_TRY,
    AST_CATCH,
    AST_PRINT,
    AST_HTTP_REQUEST,     /* request:: method url */
    AST_ASSIGNMENT,
    AST_INDEX,           /* arr[0] or dict["key"] */
    AST_MEMBER_ACCESS,    /* obj.method or obj.field */
    AST_NEW,              /* new ClassName() */
    AST_BINARY_OP,
    AST_UNARY_OP,
    AST_NUMBER,
    AST_STRING,
    AST_IDENTIFIER,
    AST_LIST,             /* [1, 2, 3] */
    AST_DICT,             /* {key: value} */
    AST_BLOCK,            /* { ... } */
    AST_INNER_BLOCK       /* << ... >> */
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
    size_t token_index;
    size_t token_count;
} Parser;

Parser* parser_create(Lexer *lexer);
void parser_destroy(Parser *parser);
ASTNode* parser_parse(Parser *parser);
ASTNode* parser_parse_program(Parser *parser);
ASTNode* parser_parse_statement(Parser *parser);
ASTNode* parser_parse_function_def(Parser *parser);
ASTNode* parser_parse_class_def(Parser *parser);
ASTNode* parser_parse_function_call(Parser *parser);
ASTNode* parser_parse_expression(Parser *parser);
ASTNode* parser_parse_block(Parser *parser);
ASTNode* parser_parse_inner_block(Parser *parser);

ASTNode* ast_create_node(ASTNodeType type);
void ast_destroy_node(ASTNode *node);
void ast_destroy_tree(ASTNode *root);

#endif /* PARSER_H */

