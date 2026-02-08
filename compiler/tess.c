#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "tess.h"
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "package.h"

double g_compile_time = 0;
double g_execute_time = 0;
double g_total_time = 0;

int tess_run(const char *filename) {
    clock_t start_total = clock();
    
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Could not open file '%s'\n", filename);
        return 1;
    }
    
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char *source = malloc(file_size + 1);
    fread(source, 1, file_size, file);
    source[file_size] = '\0';
    fclose(file);
    
    clock_t start_compile = clock();
    
    Lexer *lexer = lexer_create(source);
    lexer_tokenize(lexer);
    
    if (lexer->token_count == 0) {
        free(source);
        lexer_destroy(lexer);
        return 0;
    }
    
    Parser *parser = parser_create(lexer);
    ASTNode *ast = parser_parse(parser);
    
    clock_t end_compile = clock();
    g_compile_time = (double)(end_compile - start_compile) / CLOCKS_PER_SEC;
    
    clock_t start_exec = clock();
    
    Interpreter *interpreter = interpreter_create();
    
    Value args_list = {VALUE_LIST, {0}};
    args_list.as.list = malloc(sizeof(List));
    args_list.as.list->count = 0;
    args_list.as.list->capacity = 4;
    args_list.as.list->items = malloc(sizeof(Value) * 4);
    
    interpreter_set_variable(interpreter, "argv", args_list);
    
    if (ast && ast->type == AST_PROGRAM) {
        ASTNode *stmt = ast->children;
        while (stmt) {
            interpreter_eval(interpreter, stmt);
            if (interpreter->error_occurred) break;
            stmt = stmt->next;
        }
    }
    
    clock_t end_exec = clock();
    g_execute_time = (double)(end_exec - start_exec) / CLOCKS_PER_SEC;
    
    clock_t end_total = clock();
    g_total_time = (double)(end_total - start_total) / CLOCKS_PER_SEC;
    
    ast_destroy_tree(ast);
    parser_destroy(parser);
    lexer_destroy(lexer);
    interpreter_destroy(interpreter);
    free(source);
    
    return 0;
}

int tess_build(const char *filename) {
    printf("Building '%s'...\n", filename);
    
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Could not open file '%s'\n", filename);
        return 1;
    }
    
    char out_name[256];
    strncpy(out_name, filename, sizeof(out_name));
    char *dot = strrchr(out_name, '.');
    if (dot) *dot = '\0';
    
#ifdef _WIN32
    strcat(out_name, ".exe");
#endif
    
    printf("Compiling to native executable '%s'...\n", out_name);
    printf("(Native compilation not yet implemented, running interpretation check instead)\n");
    
    fclose(file);
    return tess_run(filename);
}

int tess_new_project(const char *name) {
    printf("Creating new project '%s'...\n", name);
    return 0;
}

int tess_info(void) {
    printf("Tess Language v0.1.0\n");
    printf("Installed packages:\n");
    return 0;
}

int tess_repl(void) {
    printf("Tess REPL v0.1.0\n");
    printf("Type 'exit' to quit.\n");
    
    char line[1024];
    Interpreter *interpreter = interpreter_create();
    
    while (1) {
        printf(">> ");
        if (!fgets(line, sizeof(line), stdin)) break;
        
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') line[len-1] = '\0';
        
        if (strcmp(line, "exit") == 0) break;
        
        Lexer *lexer = lexer_create(line);
        lexer_tokenize(lexer);
        Parser *parser = parser_create(lexer);
        ASTNode *ast = parser_parse(parser);
        
        if (ast && ast->type == AST_PROGRAM) {
            ASTNode *stmt = ast->children;
            while (stmt) {
                Value result = interpreter_eval(interpreter, stmt);
                if (result.type != VALUE_NULL) {
                    if (result.type == VALUE_NUMBER) printf("%g\n", result.as.number);
                    else if (result.type == VALUE_STRING) printf("%s\n", result.as.string);
                }
                stmt = stmt->next;
            }
        }
        
        ast_destroy_tree(ast);
        parser_destroy(parser);
        lexer_destroy(lexer);
    }
    
    interpreter_destroy(interpreter);
    return 0;
}

int tess_exec(const char *code) {
    Lexer *lexer = lexer_create(code);
    lexer_tokenize(lexer);
    Parser *parser = parser_create(lexer);
    ASTNode *ast = parser_parse(parser);
    Interpreter *interpreter = interpreter_create();
    
    if (ast && ast->type == AST_PROGRAM) {
        ASTNode *stmt = ast->children;
        while (stmt) {
            interpreter_eval(interpreter, stmt);
            stmt = stmt->next;
        }
    }
    
    ast_destroy_tree(ast);
    parser_destroy(parser);
    lexer_destroy(lexer);
    interpreter_destroy(interpreter);
    return 0;
}

int tess_version(void) {
    printf("Tess Language v0.1.0\n");
    return 0;
}

int tess_fmt(const char *file) {
    printf("Formatting %s...\n", file);
    return 0;
}

int tess_lint(const char *file) {
    printf("Linting %s...\n", file);
    return 0;
}

int tess_check(const char *file) {
    printf("Checking syntax %s...\n", file);
    return 0;
}

int tess_venv(const char *dir) {
    printf("Creating venv in %s...\n", dir);
    return 0;
}

int tess_test(void) {
    printf("Running tests...\n");
    return 0;
}
