#include "parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Parser* parser_create(Lexer *lexer) {
    Parser *parser = malloc(sizeof(Parser));
    parser->lexer = lexer;
    parser->tokens = lexer->tokens;
    parser->token_count = lexer->token_count;
    parser->token_index = 0;
    return parser;
}

void parser_destroy(Parser *parser) {
    free(parser);
}

static Token parser_current_token(Parser *parser) {
    if (parser->token_index >= parser->token_count) {
        Token eof = {TOKEN_EOF, NULL, 0, 0};
        return eof;
    }
    return parser->tokens[parser->token_index];
}

static Token parser_peek_token(Parser *parser, int offset) {
    if (parser->token_index + offset >= parser->token_count) {
        Token eof = {TOKEN_EOF, NULL, 0, 0};
        return eof;
    }
    return parser->tokens[parser->token_index + offset];
}

static void parser_advance(Parser *parser) {
    if (parser->token_index < parser->token_count) {
        parser->token_index++;
    }
}

static int parser_match(Parser *parser, TessTokenType type) {
    if (parser_current_token(parser).type == type) {
        parser_advance(parser);
        return 1;
    }
    return 0;
}

ASTNode* ast_create_node(ASTNodeType type) {
    ASTNode *node = malloc(sizeof(ASTNode));
    node->type = type;
    node->left = NULL;
    node->right = NULL;
    node->children = NULL;
    node->next = NULL;
    node->value = NULL;
    node->line = 0;
    node->column = 0;
    return node;
}

void ast_destroy_node(ASTNode *node) {
    if (!node) return;
    if (node->value) free(node->value);
    free(node);
}

void ast_destroy_tree(ASTNode *root) {
    if (!root) return;
    ast_destroy_tree(root->left);
    ast_destroy_tree(root->right);
    ast_destroy_tree(root->children);
    ast_destroy_tree(root->next);
    ast_destroy_node(root);
}

ASTNode* parser_parse_expression(Parser *parser);
ASTNode* parser_parse_statement(Parser *parser);
ASTNode* parser_parse_block(Parser *parser);
ASTNode* parser_parse_inner_block(Parser *parser);

ASTNode* parser_parse_program(Parser *parser) {
    ASTNode *program = ast_create_node(AST_PROGRAM);
    ASTNode *current = NULL;

    while (parser_current_token(parser).type != TOKEN_EOF) {
        ASTNode *stmt = parser_parse_statement(parser);
        if (stmt) {
            if (!program->children) {
                program->children = stmt;
            } else {
                current->next = stmt;
            }
            current = stmt;
        } else {
            parser_advance(parser);
        }
    }
    return program;
}

ASTNode* parser_parse(Parser *parser) {
    return parser_parse_program(parser);
}

ASTNode* parser_parse_primary(Parser *parser) {
    Token token = parser_current_token(parser);
    
    if (token.type == TOKEN_NUMBER) {
        ASTNode *node = ast_create_node(AST_NUMBER);
        node->value = strdup(token.value);
        parser_advance(parser);
        return node;
    }
    
    if (token.type == TOKEN_STRING) {
        ASTNode *node = ast_create_node(AST_STRING);
        node->value = strdup(token.value);
        parser_advance(parser);
        return node;
    }
    
    if (token.type == TOKEN_REQUEST || token.type == TOKEN_HTTP) {
        parser_advance(parser);
        ASTNode *node = ast_create_node(AST_HTTP_REQUEST);
        node->left = parser_parse_expression(parser);
        node->right = parser_parse_expression(parser);
        Token next = parser_current_token(parser);
        Token prev = parser->tokens[parser->token_index - 1];
        if (next.line == prev.line && (next.type == TOKEN_STRING || next.type == TOKEN_IDENTIFIER)) {
            node->children = parser_parse_primary(parser);
        }
        return node;
    }
    
    if (token.type == TOKEN_IDENTIFIER) {
        ASTNode *node = ast_create_node(AST_IDENTIFIER);
        node->value = strdup(token.value);
        parser_advance(parser);
        
        if (parser_current_token(parser).type == TOKEN_LPAREN) {
            parser_advance(parser);
            ASTNode *call_node = ast_create_node(AST_FUNCTION_CALL);
            call_node->value = node->value;
            free(node);
            
            ASTNode *last_arg = NULL;
            while (parser_current_token(parser).type != TOKEN_RPAREN && 
                   parser_current_token(parser).type != TOKEN_EOF) {
                ASTNode *arg = parser_parse_expression(parser);
                if (last_arg) {
                    last_arg->next = arg;
                } else {
                    call_node->left = arg;
                }
                last_arg = arg;
                
                if (parser_current_token(parser).type == TOKEN_COMMA) {
                    parser_advance(parser);
                }
            }
            parser_match(parser, TOKEN_RPAREN);
            return call_node;
        }
        
        if (parser_current_token(parser).type == TOKEN_DOT) {
            parser_advance(parser);
            Token member = parser_current_token(parser);
            if (member.type == TOKEN_IDENTIFIER) {
                ASTNode *access_node = ast_create_node(AST_MEMBER_ACCESS);
                access_node->left = node;
                
                ASTNode *member_node = ast_create_node(AST_IDENTIFIER);
                member_node->value = strdup(member.value);
                access_node->right = member_node;
                
                parser_advance(parser);
                
                if (parser_current_token(parser).type == TOKEN_LPAREN) {
                    parser_advance(parser);
                    access_node->value = strdup("call");
                    
                    ASTNode *last_arg = NULL;
                    while (parser_current_token(parser).type != TOKEN_RPAREN && 
                           parser_current_token(parser).type != TOKEN_EOF) {
                        ASTNode *arg = parser_parse_expression(parser);
                        if (last_arg) {
                            last_arg->next = arg;
                        } else {
                            access_node->children = arg;
                        }
                        last_arg = arg;
                        
                        if (parser_current_token(parser).type == TOKEN_COMMA) {
                            parser_advance(parser);
                        }
                    }
                    parser_match(parser, TOKEN_RPAREN);
                }
                
                return access_node;
            }
        }
        
        return node;
    }
    
    if (token.type == TOKEN_LPAREN) {
        parser_advance(parser);
        ASTNode *expr = parser_parse_expression(parser);
        parser_match(parser, TOKEN_RPAREN);
        return expr;
    }
    
    if (token.type == TOKEN_NEW) {
        parser_advance(parser);
        Token class_name = parser_current_token(parser);
        if (class_name.type == TOKEN_IDENTIFIER) {
            ASTNode *node = ast_create_node(AST_NEW);
            node->value = strdup(class_name.value);
            parser_advance(parser);
            
            if (parser_current_token(parser).type == TOKEN_LPAREN) {
                parser_advance(parser);
                parser_match(parser, TOKEN_RPAREN);
            }
            return node;
        }
    }

    if (token.type == TOKEN_LBRACKET) {
        parser_advance(parser);
        ASTNode *node = ast_create_node(AST_LIST);
        ASTNode *last_item = NULL;
        
        while (parser_current_token(parser).type != TOKEN_RBRACKET &&
               parser_current_token(parser).type != TOKEN_EOF) {
            ASTNode *item = parser_parse_expression(parser);
            if (last_item) {
                last_item->next = item;
            } else {
                node->children = item;
            }
            last_item = item;
            
            if (parser_current_token(parser).type == TOKEN_COMMA) {
                parser_advance(parser);
            }
        }
        parser_match(parser, TOKEN_RBRACKET);
        return node;
    }
    
    return NULL;
}

ASTNode* parser_parse_expression(Parser *parser) {
    ASTNode *left = parser_parse_primary(parser);
    
    while (parser_current_token(parser).type == TOKEN_PLUS ||
           parser_current_token(parser).type == TOKEN_MINUS ||
           parser_current_token(parser).type == TOKEN_MULTIPLY ||
           parser_current_token(parser).type == TOKEN_DIVIDE ||
           parser_current_token(parser).type == TOKEN_GT ||
           parser_current_token(parser).type == TOKEN_LT ||
           parser_current_token(parser).type == TOKEN_EQ ||
           parser_current_token(parser).type == TOKEN_PERCENT
           ) {
        TessTokenType op = parser_current_token(parser).type;
        char *op_str = NULL;
        if (op == TOKEN_PLUS) op_str = "+";
        else if (op == TOKEN_MINUS) op_str = "-";
        else if (op == TOKEN_MULTIPLY) op_str = "*";
        else if (op == TOKEN_DIVIDE) op_str = "/";
        else if (op == TOKEN_GT) op_str = ">";
        else if (op == TOKEN_LT) op_str = "<";
        
        parser_advance(parser);
        ASTNode *right = parser_parse_primary(parser);
        
        ASTNode *node = ast_create_node(AST_BINARY_OP);
        node->value = strdup(op_str ? op_str : "?");
        node->left = left;
        node->right = right;
        left = node;
    }
    
    return left;
}

ASTNode* parser_parse_function_def(Parser *parser) {
    parser_advance(parser);
    
    if (parser_current_token(parser).type == TOKEN_EXCLAMATION) {
        parser_advance(parser);
    }
    
    ASTNode *node = ast_create_node(AST_FUNCTION_DEF);
    if (parser_current_token(parser).type == TOKEN_IDENTIFIER) {
        node->value = strdup(parser_current_token(parser).value);
        parser_advance(parser);
    }
    
    if (parser_current_token(parser).type == TOKEN_LPAREN) {
        parser_advance(parser);
        ASTNode *last_param = NULL;
        while (parser_current_token(parser).type != TOKEN_RPAREN &&
               parser_current_token(parser).type != TOKEN_EOF) {
            if (parser_current_token(parser).type == TOKEN_IDENTIFIER) {
                ASTNode *param = ast_create_node(AST_VARIABLE_DECL);
                param->value = strdup(parser_current_token(parser).value);
                if (last_param) last_param->next = param;
                else node->left = param;
                last_param = param;
                parser_advance(parser);
            }
            if (parser_current_token(parser).type == TOKEN_COMMA) parser_advance(parser);
        }
        parser_match(parser, TOKEN_RPAREN);
    }
    
    if (parser_current_token(parser).type == TOKEN_LT_LT) {
        parser_advance(parser);
        node->children = parser_parse_inner_block(parser);
    } else if (parser_current_token(parser).type == TOKEN_LBRACE) {
        parser_advance(parser);
        node->children = parser_parse_block(parser);
    }
    
    return node;
}

ASTNode* parser_parse_class_def(Parser *parser) {
    parser_advance(parser);
    ASTNode *node = ast_create_node(AST_CLASS_DEF);
    if (parser_current_token(parser).type == TOKEN_IDENTIFIER) {
        node->value = strdup(parser_current_token(parser).value);
        parser_advance(parser);
    }
    
    if (parser_match(parser, TOKEN_LBRACE)) {
        ASTNode *last_member = NULL;
        while (parser_current_token(parser).type != TOKEN_RBRACE &&
               parser_current_token(parser).type != TOKEN_EOF) {
            ASTNode *member = NULL;
            if (parser_current_token(parser).type == TOKEN_IDENTIFIER && 
                strcmp(parser_current_token(parser).value, "f") == 0) {
                 member = parser_parse_function_def(parser);
            } else {
                 parser_advance(parser);
            }
            
            if (member) {
                if (last_member) last_member->next = member;
                else node->children = member;
                last_member = member;
            }
        }
        parser_match(parser, TOKEN_RBRACE);
    }
    return node;
}

ASTNode* parser_parse_statement(Parser *parser) {
    Token token = parser_current_token(parser);
    
    if (token.type == TOKEN_IDENTIFIER && strcmp(token.value, "f") == 0) {
        if (parser_peek_token(parser, 1).type == TOKEN_EXCLAMATION) {
            return parser_parse_function_def(parser);
        }
    }
    
    if (token.type == TOKEN_CLASS) {
        return parser_parse_class_def(parser);
    }
    
    if (token.type == TOKEN_RETURN) {
        parser_advance(parser);
        ASTNode *node = ast_create_node(AST_RETURN);
        node->left = parser_parse_expression(parser);
        return node;
    }
    
    if (token.type == TOKEN_PRINT) {
        parser_advance(parser);
        ASTNode *node = ast_create_node(AST_PRINT);
        node->left = parser_parse_expression(parser);
        
        ASTNode *last = node->left;
        while (parser_current_token(parser).type == TOKEN_COMMA) {
            parser_advance(parser);
            ASTNode *next_expr = parser_parse_expression(parser);
            if (last) last->next = next_expr;
            last = next_expr;
        }
        return node;
    }
    
    if (token.type == TOKEN_REQUEST || token.type == TOKEN_HTTP) {
        parser_advance(parser);
        ASTNode *node = ast_create_node(AST_HTTP_REQUEST);
        node->left = parser_parse_expression(parser);
        node->right = parser_parse_expression(parser);
        return node;
    }
    
    if (token.type == TOKEN_WHILE) {
        parser_advance(parser);
        ASTNode *node = ast_create_node(AST_WHILE);
        node->left = parser_parse_expression(parser);
        parser_match(parser, TOKEN_LBRACE);
        node->children = parser_parse_block(parser);
        return node;
    }
    
    if (token.type == TOKEN_IF) {
        parser_advance(parser);
        ASTNode *node = ast_create_node(AST_IF);
        node->left = parser_parse_expression(parser);
        parser_match(parser, TOKEN_LBRACE);
        node->children = parser_parse_block(parser);
        if (parser_current_token(parser).type == TOKEN_ELSE) {
            parser_advance(parser);
            parser_match(parser, TOKEN_LBRACE);
            node->right = parser_parse_block(parser);
        }
        return node;
    }
    
    if (token.type == TOKEN_GET) {
        parser_advance(parser);
        Token module_token = parser_current_token(parser);
        if (module_token.type == TOKEN_IDENTIFIER) {
            ASTNode *node = ast_create_node(AST_IMPORT);
            node->value = strdup(module_token.value);
            parser_advance(parser);
            return node;
        }
        return NULL;
    }

    if (token.type == TOKEN_ADD) {
        parser_advance(parser);
        Token module_token = parser_current_token(parser);
        if (module_token.type == TOKEN_IDENTIFIER) {
            ASTNode *node = ast_create_node(AST_IMPORT);
            char module_path[512] = {0};
            strcpy(module_path, module_token.value);
            parser_advance(parser);
            
            Token next = parser_current_token(parser);
            if (next.type == TOKEN_IDENTIFIER && strcmp(next.value, "from") == 0) {
                parser_advance(parser);
                Token from_module = parser_current_token(parser);
                if (from_module.type == TOKEN_IDENTIFIER) {
                    snprintf(module_path, sizeof(module_path), "%s/%s", from_module.value, module_token.value);
                    parser_advance(parser);
                }
            }
            next = parser_current_token(parser);
            if (next.type == TOKEN_IDENTIFIER && strcmp(next.value, "as") == 0) {
                parser_advance(parser);
                Token alias = parser_current_token(parser);
                if (alias.type == TOKEN_IDENTIFIER || alias.type == TOKEN_NUMBER) {
                    node->right = ast_create_node(AST_IDENTIFIER);
                    node->right->value = strdup(alias.value);
                    parser_advance(parser);
                }
            }
            
            node->value = strdup(module_path);
            return node;
        }
        return NULL;
    }

    if (token.type == TOKEN_START) {
        parser_advance(parser);
        
        if (parser_current_token(parser).type == TOKEN_GT) {
            parser_advance(parser);
            
            char *entry_point = "main";
            
            if (parser_current_token(parser).type == TOKEN_DOT) {
                parser_advance(parser);
            } else if (parser_current_token(parser).type == TOKEN_IDENTIFIER) {
                entry_point = parser_current_token(parser).value;
                parser_advance(parser);
            }
            
            if (parser_current_token(parser).type == TOKEN_LT) {
                parser_advance(parser);
            }
            
            ASTNode *node = ast_create_node(AST_MAIN_CALL);
            node->value = strdup(entry_point);
            return node;
        }
    }
    
    if (token.type == TOKEN_IDENTIFIER) {
        if (parser_peek_token(parser, 1).type == TOKEN_ASSIGN) {
            ASTNode *node = ast_create_node(AST_ASSIGNMENT);
            node->value = strdup(token.value);
            parser_advance(parser);
            parser_advance(parser);
            node->right = parser_parse_expression(parser);
            return node;
        }
        
        if (parser_peek_token(parser, 1).type == TOKEN_LPAREN) {
            return parser_parse_expression(parser);
        }
        
        if (parser_peek_token(parser, 1).type == TOKEN_DOT) {
             return parser_parse_expression(parser);
        }
    }
    
    if (token.type == TOKEN_REPEAT) {
        parser_advance(parser);
        ASTNode *node = ast_create_node(AST_REPEAT);
        node->left = parser_parse_expression(parser);
        if (parser_current_token(parser).type == TOKEN_LBRACE) {
            parser_advance(parser);
            node->children = parser_parse_block(parser);
        } else if (parser_current_token(parser).type == TOKEN_LT_LT) {
            parser_advance(parser);
            node->children = parser_parse_inner_block(parser);
        }
        return node;
    }
    
    if (token.type == TOKEN_TRY) {
        parser_advance(parser);
        ASTNode *node = ast_create_node(AST_TRY);
        
        if (parser_match(parser, TOKEN_LBRACE)) {
            node->left = parser_parse_block(parser);
        }
        
        if (parser_current_token(parser).type == TOKEN_CATCH) {
            parser_advance(parser);
            if (parser_match(parser, TOKEN_LBRACE)) {
                node->right = parser_parse_block(parser);
            }
        }
        return node;
    }
    
    return NULL;
}

ASTNode* parser_parse_block(Parser *parser) {
    ASTNode *block = ast_create_node(AST_BLOCK);
    ASTNode *current = NULL;
    
    while (parser_current_token(parser).type != TOKEN_RBRACE && 
           parser_current_token(parser).type != TOKEN_EOF) {
        ASTNode *stmt = parser_parse_statement(parser);
        if (stmt) {
            if (!block->children) {
                block->children = stmt;
            } else {
                current->next = stmt;
            }
            current = stmt;
        } else {
            parser_advance(parser);
        }
    }
    parser_match(parser, TOKEN_RBRACE);
    return block;
}

ASTNode* parser_parse_inner_block(Parser *parser) {
    ASTNode *block = ast_create_node(AST_INNER_BLOCK);
    ASTNode *current = NULL;
    
    while (parser_current_token(parser).type != TOKEN_GT_GT && 
           parser_current_token(parser).type != TOKEN_EOF) {
        ASTNode *stmt = parser_parse_statement(parser);
        if (stmt) {
            if (!block->children) {
                block->children = stmt;
            } else {
                current->next = stmt;
            }
            current = stmt;
        } else {
            parser_advance(parser);
        }
    }
    parser_match(parser, TOKEN_GT_GT);
    return block;
}
