#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#include <io.h>
#define access _access
#define F_OK 0
#else
#include <unistd.h>
#endif
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"

#define MODULE_EXT ".tess"
#define PACKAGE_DIR ".tess_packages"
#define SAINT_DIR "SAINT"

static char* find_module_file(const char *module_name) {
    static char path[1024];
    
    /* Try current directory */
    snprintf(path, sizeof(path), "%s%s", module_name, MODULE_EXT);
    if (access(path, F_OK) == 0) {
        return path;
    }
    
    /* Try SAINT folder (for library modules) */
    snprintf(path, sizeof(path), "%s/%s%s", SAINT_DIR, module_name, MODULE_EXT);
    if (access(path, F_OK) == 0) {
        return path;
    }
    
    /* Try SAINT/module_name/module_name.tess (folder structure) */
    snprintf(path, sizeof(path), "%s/%s/%s%s", SAINT_DIR, module_name, module_name, MODULE_EXT);
    if (access(path, F_OK) == 0) {
        return path;
    }
    
    /* Try nested module path (e.g., "saint/micheal") */
    char *slash = strchr(module_name, '/');
    if (slash) {
        snprintf(path, sizeof(path), "%s/%s%s", SAINT_DIR, module_name, MODULE_EXT);
        if (access(path, F_OK) == 0) {
            return path;
        }
    }
    
    /* Try package directory */
    snprintf(path, sizeof(path), "%s/%s%s", PACKAGE_DIR, module_name, MODULE_EXT);
    if (access(path, F_OK) == 0) {
        return path;
    }
    
    return NULL;
}

ASTNode* module_load(const char *module_name) {
    char *module_file = find_module_file(module_name);
    if (!module_file) {
        fprintf(stderr, "Error: Module '%s' not found\n", module_name);
        return NULL;
    }
    
    FILE *file = fopen(module_file, "r");
    if (!file) {
        fprintf(stderr, "Error: Cannot open module file '%s'\n", module_file);
        return NULL;
    }
    
    /* Read file */
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char *source = malloc(file_size + 1);
    fread(source, 1, file_size, file);
    source[file_size] = '\0';
    fclose(file);
    
    /* Parse module */
    Lexer *lexer = lexer_create(source);
    lexer_tokenize(lexer);
    
    if (!lexer->tokens || lexer->token_count == 0) {
        fprintf(stderr, "Error: Failed to tokenize module '%s'\n", module_name);
        free(source);
        lexer_destroy(lexer);
        return NULL;
    }
    
    Parser *parser = parser_create(lexer);
    ASTNode *ast = parser_parse(parser);
    
    free(source);
    lexer_destroy(lexer);
    parser_destroy(parser);
    
    return ast;
}

void module_execute(Interpreter *interpreter, ASTNode *module_ast) {
    if (!module_ast || module_ast->type != AST_PROGRAM) {
        return;
    }
    
    /* Push new scope for module */
    interpreter_push_scope(interpreter);
    
    /* Execute module statements */
    ASTNode *stmt = module_ast->children;
    while (stmt) {
        interpreter_eval(interpreter, stmt);
        stmt = stmt->next;
    }
    
    /* Note: We don't pop scope - module variables stay in scope */
}

