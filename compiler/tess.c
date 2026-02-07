#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <errno.h>
#ifdef _WIN32
#include <direct.h>
#include <io.h>
#define stat _stat
#define access _access
#define F_OK 0
#define mkdir(dir) _mkdir(dir)
#else
#include <unistd.h>
#include <sys/types.h>
#endif
#include "tess.h"
#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "config.h"

/* Global timing data - accessible via timing() builtin function */
double g_compile_time = 0.0;
double g_execute_time = 0.0;
double g_total_time = 0.0;

/* Helper function to find .tess.stn file */
static char* find_config_file(const char *tess_file) {
    static char config_path[1024];
    
    /* Try same directory as tess file */
    strncpy(config_path, tess_file, sizeof(config_path) - 1);
    char *last_slash = strrchr(config_path, '/');
    if (!last_slash) {
        last_slash = strrchr(config_path, '\\');
    }
    if (last_slash) {
        *(last_slash + 1) = '\0';
    } else {
        config_path[0] = '\0';
    }
    strcat(config_path, ".tess.stn");
    
    if (access(config_path, F_OK) == 0) {
        return config_path;
    }
    
    /* Try current directory */
    strcpy(config_path, ".tess.stn");
    if (access(config_path, F_OK) == 0) {
        return config_path;
    }
    
    return NULL;
}

int tess_run(const char *filename) {
    clock_t start_total = clock();
    clock_t start_compile, end_compile;
    clock_t start_execute, end_execute;
    double compile_time, execute_time, total_time;
    
    /* Auto-add .tess extension if not present */
    char actual_filename[512];
    strncpy(actual_filename, filename, sizeof(actual_filename) - 1);
    actual_filename[sizeof(actual_filename) - 1] = '\0';
    
    /* Check if file exists as-is */
    FILE *test_file = fopen(actual_filename, "r");
    if (!test_file) {
        /* Try adding .tess extension */
        size_t len = strlen(actual_filename);
        if (len < sizeof(actual_filename) - 5) {
            strcat(actual_filename, ".tess");
        }
    } else {
        fclose(test_file);
    }
    
    /* Load .tess.stn config file if it exists */
    ConfigMap config = {0};
    char *config_file = find_config_file(actual_filename);
    if (config_file) {
        config_load_file(config_file, &config);
    }
    
    FILE *file = fopen(actual_filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file '%s' (also tried '%s.tess')\n", filename, filename);
        config_destroy(&config);
        return 1;
    }

    /* Read file into memory */
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *source = malloc(file_size + 1);
    size_t read_size = fread(source, 1, file_size, file);
    source[read_size] = '\0';
    fclose(file);

    /* Start compilation timing */
    start_compile = clock();

    /* Lexical analysis */
    /* printf("Debug: Lexing...\n"); */
    Lexer *lexer = lexer_create(source);
    lexer_tokenize(lexer);
    
    if (lexer->token_count == 0) {
        fprintf(stderr, "Error: Lexical analysis failed or empty file\n");
        free(source);
        lexer_destroy(lexer);
        return 1;
    }

    /* Parsing */
    /* printf("Debug: Parsing...\n"); */
    Parser *parser = parser_create(lexer);
    ASTNode *ast = parser_parse(parser);
    
    if (!ast) {
        fprintf(stderr, "Error: Parsing failed\n");
        free(source);
        lexer_destroy(lexer);
        parser_destroy(parser);
        return 1;
    }

    /* End compilation timing */
    end_compile = clock();
    compile_time = ((double)(end_compile - start_compile)) / CLOCKS_PER_SEC;

    /* Start execution timing */
    start_execute = clock();

    /* Interpretation */
    /* printf("Debug: Executing...\n"); */
    Interpreter *interpreter = interpreter_create();
    interpreter_eval(interpreter, ast);

    /* End execution timing */
    end_execute = clock();
    execute_time = ((double)(end_execute - start_execute)) / CLOCKS_PER_SEC;

    /* Cleanup */
    free(source);
    lexer_destroy(lexer);
    parser_destroy(parser);
    ast_destroy_tree(ast);
    interpreter_destroy(interpreter);
    config_destroy(&config);

    /* Calculate total time */
    clock_t end_total = clock();
    total_time = ((double)(end_total - start_total)) / CLOCKS_PER_SEC;

    /* Store timing information globally for access via timing() function */
    g_compile_time = compile_time;
    g_execute_time = execute_time;
    g_total_time = total_time;

    /* Timing information is now available via timing() function - no automatic printing */

    return 0;
}

int tess_build(const char *filename) {
    /* TODO: Implement compilation to C or binary */
    printf("Building %s...\n", filename);
    printf("(Compilation not yet implemented)\n");
    return 0;
}

int tess_new_project(const char *project_name) {
    if (!project_name) {
        fprintf(stderr, "Error: No project name specified\n");
        return 1;
    }

    printf("Creating new project '%s'...\n", project_name);

    /* Create project directory */
#ifdef _WIN32
    if (_mkdir(project_name) != 0) {
        if (errno != EEXIST) {
            fprintf(stderr, "Error: Cannot create directory '%s'\n", project_name);
            return 1;
        }
    }
#else
    if (mkdir(project_name, 0755) != 0) {
        if (errno != EEXIST) {
            fprintf(stderr, "Error: Cannot create directory '%s'\n", project_name);
            return 1;
        }
    }
#endif

    /* Create main.tess */
    char main_file_path[1024];
    snprintf(main_file_path, sizeof(main_file_path), "%s/main.tess", project_name);
    
    FILE *f = fopen(main_file_path, "w");
    if (!f) {
        fprintf(stderr, "Error: Cannot create file '%s'\n", main_file_path);
        return 1;
    }
    
    fprintf(f, "f! main {\n");
    fprintf(f, "    print:: \"Hello, World!\"\n");
    fprintf(f, "}\n\n");
    fprintf(f, "start >.<\n");
    fclose(f);
    
    /* Create .tess_packages directory */
    char pkg_dir[1024];
    snprintf(pkg_dir, sizeof(pkg_dir), "%s/.tess_packages", project_name);
#ifdef _WIN32
    _mkdir(pkg_dir);
#else
    mkdir(pkg_dir, 0755);
#endif

    printf("Project created successfully!\n");
    printf("Run 'cd %s' and 'tess run main.tess' to get started.\n", project_name);
    
    return 0;
}

int tess_info(void) {
    printf("Tess Language v0.1.0\n");
    printf("(Package info not yet implemented)\n");
    return 0;
}

/* REPL - Interactive shell */
int tess_repl(void) {
    printf("Tess REPL (Interactive Shell)\n");
    printf("Type 'exit' or 'quit' to exit\n");
    printf("Note: Enter one command per line. Variables persist.\n");
    printf(">>> ");
    fflush(stdout);
    
    Interpreter *interpreter = interpreter_create();
    
    char line[4096];
    while (fgets(line, sizeof(line), stdin)) {
        /* Remove newline */
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }
        
        /* Check for exit */
        if (strcmp(line, "exit") == 0 || strcmp(line, "quit") == 0) {
            break;
        }
        
        if (strlen(line) == 0) {
            printf(">>> ");
            fflush(stdout);
            continue;
        }
        
        /* Execute the code */
        Lexer *lexer = lexer_create(line);
        lexer_tokenize(lexer);
        
        Parser *parser = parser_create(lexer);
        ASTNode *ast = parser_parse(parser);
        
        if (ast) {
            interpreter_eval(interpreter, ast);
            ast_destroy_tree(ast);
        }
        
        lexer_destroy(lexer);
        parser_destroy(parser);
        
        printf(">>> ");
        fflush(stdout);
    }
    
    interpreter_destroy(interpreter);
    return 0;
}

/* Execute inline code */
int tess_exec(const char *code) {
    if (!code || strlen(code) == 0) {
        fprintf(stderr, "Error: No code provided\n");
        return 1;
    }
    
    Lexer *lexer = lexer_create(code);
    lexer_tokenize(lexer);
    
    Parser *parser = parser_create(lexer);
    ASTNode *ast = parser_parse(parser);
    
    if (ast) {
        Interpreter *interpreter = interpreter_create();
        interpreter_eval(interpreter, ast);
        interpreter_destroy(interpreter);
        ast_destroy_tree(ast);
    }
    
    lexer_destroy(lexer);
    parser_destroy(parser);
    
    return 0;
}

/* Show version */
int tess_version(void) {
    printf("Tess Language v0.1.0-beta\n");
    return 0;
}

/* Formatter */
int tess_fmt(const char *filename) {
    (void)filename;
    /* TODO: Implement formatter */
    printf("Formatter not yet implemented\n");
    return 0;
}

/* Linter */
int tess_lint(const char *filename) {
    (void)filename;
    /* TODO: Implement linter */
    printf("Linter not yet implemented\n");
    return 0;
}

/* Syntax check only */
int tess_check(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", filename);
        return 1;
    }
    
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char *source = malloc(file_size + 1);
    size_t read_size = fread(source, 1, file_size, file);
    source[read_size] = '\0';
    fclose(file);
    
    Lexer *lexer = lexer_create(source);
    lexer_tokenize(lexer);
    
    Parser *parser = parser_create(lexer);
    ASTNode *ast = parser_parse(parser);
    
    if (ast) {
        printf("Syntax check passed: %s\n", filename);
        ast_destroy_tree(ast);
    } else {
        fprintf(stderr, "Syntax error in %s\n", filename);
        free(source);
        lexer_destroy(lexer);
        parser_destroy(parser);
        return 1;
    }
    
    free(source);
    lexer_destroy(lexer);
    parser_destroy(parser);
    
    return 0;
}

/* Virtual environment creator */
int tess_venv(const char *dir) {
    if (!dir) {
        fprintf(stderr, "Error: No directory specified\n");
        return 1;
    }
    
#ifdef _WIN32
    if (_mkdir(dir) != 0) {
        if (errno != EEXIST) {
            fprintf(stderr, "Error: Cannot create directory '%s'\n", dir);
            return 1;
        }
    }
#else
    if (mkdir(dir, 0755) != 0) {
        if (errno != EEXIST) {
            fprintf(stderr, "Error: Cannot create directory '%s'\n", dir);
            return 1;
        }
    }
#endif
    
    printf("Virtual environment created at: %s\n", dir);
    return 0;
}

/* Test runner */
int tess_test(void) {
    /* TODO: Implement test runner */
    printf("Test runner not yet implemented\n");
    return 0;
}
