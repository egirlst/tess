#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tess.h"

int main_tess(int argc, char *argv[]) {
    setbuf(stdout, NULL);
    if (argc < 2) {
        printf("Tess Language Compiler/Interpreter\n");
        printf("Usage: tess <command> [arguments]\n");
        printf("\nCommands:\n");
        printf("  run <file>    - Run a .tess file (alias: r)\n");
        printf("  build <file>  - Compile a .tess file (alias: b)\n");
        printf("  install <pkg> - Install a package (aliases: ins, i)\n");
        printf("  i .           - Install local package from .tess.noah\n");
        printf("  uninstall     - Remove a package (aliases: del, remove)\n");
        printf("  new <project> - Create a new project skeleton\n");
        printf("  info          - Show info about installed packages/version\n");
        printf("  update <pkg>  - Update a package\n");
        printf("  repl          - Interactive shell (Python style)\n");
        printf("  exec <code>   - Run inline code (Python -c style)\n");
        printf("  version       - Show version\n");
        printf("  fmt <file>    - Format code (like black)\n");
        printf("  lint <file>   - Lint code (like flake8/ruff)\n");
        printf("  check <file>  - Syntax check only (Python py_compile style)\n");
        printf("  venv <dir>    - Create virtual environment (Python -m venv style)\n");
        printf("  test          - Run tests (pytest equivalent)\n");
        printf("\nNote: 'ts' can be used as alias for 'tess'\n");
        return 1;
    }

    char *command = argv[1];

    if (strcmp(command, "run") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: No file specified\n");
            return 1;
        }
        return tess_run(argv[2]);
    } else if (strcmp(command, "build") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: No file specified\n");
            return 1;
        }
        return tess_build(argv[2]);
    } else if (strcmp(command, "install") == 0 || strcmp(command, "ins") == 0 || 
               strcmp(command, "i") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: No package specified\n");
            fprintf(stderr, "Usage: tess install <package> or tess i . (for local)\n");
            return 1;
        }
        return tess_install(argv[2]);
    } else if (strcmp(command, "r") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: No file specified\n");
            return 1;
        }
        return tess_run(argv[2]);
    } else if (strcmp(command, "b") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: No file specified\n");
            return 1;
        }
        return tess_build(argv[2]);
    } else if (strcmp(command, "uninstall") == 0 || strcmp(command, "del") == 0 || 
               strcmp(command, "remove") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: No package specified\n");
            return 1;
        }
        return tess_uninstall(argv[2]);
    } else if (strcmp(command, "new") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: No project name specified\n");
            return 1;
        }
        return tess_new_project(argv[2]);
    } else if (strcmp(command, "info") == 0) {
        return tess_info();
    } else if (strcmp(command, "update") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: No package specified\n");
            return 1;
        }
        return tess_update(argv[2]);
    } else if (strcmp(command, "repl") == 0) {
        return tess_repl();
    } else if (strcmp(command, "exec") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: No code specified\n");
            return 1;
        }
        return tess_exec(argv[2]);
    } else if (strcmp(command, "version") == 0) {
        return tess_version();
    } else if (strcmp(command, "fmt") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: No file specified\n");
            return 1;
        }
        return tess_fmt(argv[2]);
    } else if (strcmp(command, "lint") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: No file specified\n");
            return 1;
        }
        return tess_lint(argv[2]);
    } else if (strcmp(command, "check") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: No file specified\n");
            return 1;
        }
        return tess_check(argv[2]);
    } else if (strcmp(command, "venv") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: No directory specified\n");
            return 1;
        }
        return tess_venv(argv[2]);
    } else if (strcmp(command, "test") == 0) {
        return tess_test();
    } else {
        fprintf(stderr, "Error: Unknown command '%s'\n", command);
        return 1;
    }
}

/* Alias main for ts command */
int main(int argc, char *argv[]) {
    return main_tess(argc, argv);
}

