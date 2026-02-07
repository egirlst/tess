#include "tess_stdlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/mman.h>
#endif

/* ... existing functions ... */

Value stdlib_print(Value *args, int argc) {
    for (int i = 0; i < argc; i++) {
        switch (args[i].type) {
            case VALUE_NUMBER:
                printf("%g", args[i].as.number);
                break;
            case VALUE_STRING:
                printf("%s", args[i].as.string);
                break;
            case VALUE_NULL:
                printf("null");
                break;
            case VALUE_BOOLEAN:
                printf(args[i].as.boolean ? "true" : "false");
                break;
            case VALUE_LIST:
                printf("[List]");
                break;
            case VALUE_DICT:
                printf("[Dict]");
                break;
            case VALUE_FUNCTION:
                printf("[Function]");
                break;
            case VALUE_CLASS:
                printf("[Class]");
                break;
            case VALUE_OBJECT:
                printf("[Object]");
                break;
            case VALUE_FILE:
                printf("[File Handle: %p]", args[i].as.file);
                break;
            default:
                printf("[Unknown: %d]", args[i].type);
        }
        if (i < argc - 1) printf(" ");
    }
    printf("\n");
    return (Value){VALUE_NULL, {0}};
}

/* ... keep existing functions ... */

Value stdlib_sqrt(Value *args, int argc) {
    if (argc != 1 || args[0].type != VALUE_NUMBER) {
        return (Value){VALUE_NULL, {0}};
    }
    return (Value){VALUE_NUMBER, .as.number = sqrt(args[0].as.number)};
}

Value stdlib_read_file(Value *args, int argc) {
    if (argc != 1 || args[0].type != VALUE_STRING) {
        printf("Error: read_file expects a single string argument\n");
        return (Value){VALUE_NULL, {0}};
    }

    char *filename = args[0].as.string;
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Error: Could not open file '%s'\n", filename);
        return (Value){VALUE_NULL, {0}};
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = malloc(length + 1);
    if (!buffer) {
        fclose(file);
        return (Value){VALUE_NULL, {0}};
    }

    fread(buffer, 1, length, file);
    buffer[length] = '\0';
    fclose(file);

    Value result;
    result.type = VALUE_STRING;
    result.as.string = buffer;
    return result;
}

Value stdlib_write_file(Value *args, int argc) {
    if (argc != 2 || args[0].type != VALUE_STRING || args[1].type != VALUE_STRING) {
        printf("Error: write_file expects (filename, content)\n");
        return (Value){VALUE_NULL, {0}};
    }

    char *filename = args[0].as.string;
    char *content = args[1].as.string;
    
    FILE *file = fopen(filename, "w");
    if (!file) {
        printf("Error: Could not open file '%s' for writing\n", filename);
        return (Value){VALUE_NULL, {0}};
    }

    fputs(content, file);
    fclose(file);

    return (Value){VALUE_NULL, {0}};
}

Value stdlib_len(Value *args, int argc) {
    if (argc != 1) {
        return (Value){VALUE_NUMBER, .as.number = 0};
    }
    
    if (args[0].type == VALUE_STRING) {
        return (Value){VALUE_NUMBER, .as.number = strlen(args[0].as.string)};
    } else if (args[0].type == VALUE_LIST) {
        return (Value){VALUE_NUMBER, .as.number = args[0].as.list->count};
    } else if (args[0].type == VALUE_DICT) {
        return (Value){VALUE_NUMBER, .as.number = args[0].as.dict->count};
    }
    
    return (Value){VALUE_NUMBER, .as.number = 0};
}

Value stdlib_json_format(Value *args, int argc) {
    if (argc != 1 || args[0].type != VALUE_STRING) {
        return (Value){VALUE_NULL, {0}};
    }
    
    char *json = args[0].as.string;
    size_t len = strlen(json);
    
    /* Estimate new size (roughly 2x to accommodate indentation) */
    char *pretty = malloc(len * 4 + 1024);
    if (!pretty) return (Value){VALUE_NULL, {0}};
    
    int indent_level = 0;
    int in_string = 0;
    size_t j = 0;
    
    for (size_t i = 0; i < len; i++) {
        char c = json[i];
        
        if (c == '"' && (i == 0 || json[i-1] != '\\')) {
            in_string = !in_string;
            pretty[j++] = c;
            continue;
        }
        
        if (in_string) {
            pretty[j++] = c;
            continue;
        }
        
        if (c == '{' || c == '[') {
            pretty[j++] = c;
            pretty[j++] = '\n';
            indent_level++;
            for (int k = 0; k < indent_level; k++) {
                pretty[j++] = ' ';
                pretty[j++] = ' ';
            }
        } else if (c == '}' || c == ']') {
            pretty[j++] = '\n';
            indent_level--;
            for (int k = 0; k < indent_level; k++) {
                pretty[j++] = ' ';
                pretty[j++] = ' ';
            }
            pretty[j++] = c;
        } else if (c == ',') {
            pretty[j++] = c;
            pretty[j++] = '\n';
            for (int k = 0; k < indent_level; k++) {
                pretty[j++] = ' ';
                pretty[j++] = ' ';
            }
        } else if (c == ':') {
            pretty[j++] = c;
            pretty[j++] = ' ';
        } else if (c == ' ' || c == '\n' || c == '\t' || c == '\r') {
            /* Skip whitespace outside strings */
        } else {
            pretty[j++] = c;
        }
    }
    
    pretty[j] = '\0';
    
    return (Value){VALUE_STRING, .as.string = pretty};
}

Value stdlib_clock(Value *args, int argc) {
    (void)args;
    (void)argc;
    return (Value){VALUE_NUMBER, .as.number = (double)clock() / CLOCKS_PER_SEC};
}

/* Memory Management */

Value stdlib_mem_alloc(Value *args, int argc) {
    int arg_idx = 0;
    if (argc > 0 && args[0].type == VALUE_OBJECT) arg_idx = 1;

    if (argc < arg_idx + 1 || args[arg_idx].type != VALUE_NUMBER) {
        printf("Error: mem.alloc expects size\n");
        return (Value){VALUE_NULL, {0}};
    }
    size_t size = (size_t)args[arg_idx].as.number;
    void *ptr = malloc(size);
    if (!ptr) {
        printf("Error: malloc failed for size %lu\n", (unsigned long)size);
        return (Value){VALUE_NULL, {0}};
    }
    
    Value result;
    result.type = VALUE_FILE; /* Reusing VALUE_FILE as pointer wrapper */
    result.as.file = ptr;
    return result;
}

Value stdlib_mem_free(Value *args, int argc) {
    int arg_idx = 0;
    if (argc > 0 && args[0].type == VALUE_OBJECT) arg_idx = 1;

    if (argc < arg_idx + 1 || args[arg_idx].type != VALUE_FILE) {
        printf("Error: mem.free expects a pointer\n");
        return (Value){VALUE_NULL, {0}};
    }
    free(args[arg_idx].as.file);
    args[arg_idx].as.file = NULL;
    return (Value){VALUE_NULL, {0}};
}

Value stdlib_mem_write(Value *args, int argc) {
    /* mem.write(ptr, offset, byte_value) */
    int arg_idx = 0;
    if (argc > 0 && args[0].type == VALUE_OBJECT) arg_idx = 1;

    if (argc < arg_idx + 3 || args[arg_idx].type != VALUE_FILE || args[arg_idx+1].type != VALUE_NUMBER || args[arg_idx+2].type != VALUE_NUMBER) {
        printf("Error: mem.write expects (ptr, offset, value)\n");
        return (Value){VALUE_NULL, {0}};
    }
    
    unsigned char *ptr = (unsigned char *)args[arg_idx].as.file;
    int offset = (int)args[arg_idx+1].as.number;
    unsigned char val = (unsigned char)args[arg_idx+2].as.number;
    
    if (ptr) ptr[offset] = val;
    
    return (Value){VALUE_NULL, {0}};
}

Value stdlib_mem_read(Value *args, int argc) {
    /* mem.read(ptr, offset) -> byte */
    int arg_idx = 0;
    if (argc > 0 && args[0].type == VALUE_OBJECT) arg_idx = 1;

    if (argc < arg_idx + 2 || args[arg_idx].type != VALUE_FILE || args[arg_idx+1].type != VALUE_NUMBER) {
        printf("Error: mem.read expects (ptr, offset)\n");
        return (Value){VALUE_NULL, {0}};
    }
    
    unsigned char *ptr = (unsigned char *)args[arg_idx].as.file;
    int offset = (int)args[arg_idx+1].as.number;
    
    if (!ptr) return (Value){VALUE_NULL, {0}};
    
    return (Value){VALUE_NUMBER, .as.number = (double)ptr[offset]};
}

/* File Object Methods Implementation */

Value stdlib_file_open(Value *args, int argc) {
    /* Expects (filename, mode) or (this, filename, mode) */
    int arg_idx = 0;
    if (argc > 0 && args[0].type == VALUE_OBJECT) {
        arg_idx = 1;
    }

    if (argc < arg_idx + 1 || args[arg_idx].type != VALUE_STRING) {
        printf("Error: f.open expects at least a filename\n");
        return (Value){VALUE_NULL, {0}};
    }
    
    char *filename = args[arg_idx].as.string;
    char *mode = "r"; /* Default to read */
    if (argc >= arg_idx + 2 && args[arg_idx+1].type == VALUE_STRING) {
        mode = args[arg_idx+1].as.string;
    }
    
    FILE *f = fopen(filename, mode);
    if (!f) {
        /* TODO: Return error or throw exception? For now return NULL */
        printf("Error: Failed to open file '%s'\n", filename);
        return (Value){VALUE_NULL, {0}};
    }
    
    Value result;
    result.type = VALUE_FILE;
    result.as.file = f;
    return result;
}

Value stdlib_file_close(Value *args, int argc) {
    /* Expects (file_obj) */
    int arg_idx = 0;
    if (argc > 0 && args[0].type == VALUE_OBJECT) arg_idx = 1;

    if (argc < arg_idx + 1 || args[arg_idx].type != VALUE_FILE) {
        printf("Error: close expects a file object\n");
        return (Value){VALUE_NULL, {0}};
    }
    
    FILE *f = (FILE*)args[arg_idx].as.file;
    if (f) {
        fclose(f);
        args[arg_idx].as.file = NULL; /* Prevent double close */
    }
    
    return (Value){VALUE_NULL, {0}};
}

Value stdlib_file_write(Value *args, int argc) {
    /* Expects (file_obj, content) */
    int arg_idx = 0;
    if (argc > 0 && args[0].type == VALUE_OBJECT) arg_idx = 1;

    if (argc < arg_idx + 2 || args[arg_idx].type != VALUE_FILE || args[arg_idx+1].type != VALUE_STRING) {
        printf("Error: write expects (content)\n");
        return (Value){VALUE_NULL, {0}};
    }
    
    FILE *f = (FILE*)args[arg_idx].as.file;
    char *content = args[arg_idx+1].as.string;
    
    if (f) {
        fputs(content, f);
    } else {
        printf("Error: Writing to closed file\n");
    }
    
    return (Value){VALUE_NULL, {0}};
}

Value stdlib_file_read(Value *args, int argc) {
    /* Expects (file_obj) */
    int arg_idx = 0;
    if (argc > 0 && args[0].type == VALUE_OBJECT) arg_idx = 1;

    if (argc < arg_idx + 1 || args[arg_idx].type != VALUE_FILE) {
        printf("Error: read expects a file object\n");
        return (Value){VALUE_NULL, {0}};
    }
    
    FILE *f = (FILE*)args[arg_idx].as.file;
    if (!f) return (Value){VALUE_NULL, {0}};
    
    long current_pos = ftell(f);
    fseek(f, 0, SEEK_END);
    long length = ftell(f) - current_pos;
    fseek(f, current_pos, SEEK_SET);
    
    if (length <= 0) return (Value){VALUE_STRING, .as.string = strdup("")};

    char *buffer = malloc(length + 1);
    fread(buffer, 1, length, f);
    buffer[length] = '\0';
    
    return (Value){VALUE_STRING, .as.string = buffer};
}

/* System Methods */
Value stdlib_sys_sleep(Value *args, int argc) {
    int arg_idx = 0;
    if (argc > 0 && args[0].type == VALUE_OBJECT) arg_idx = 1;
    
    if (argc < arg_idx + 1 || args[arg_idx].type != VALUE_NUMBER) {
        return (Value){VALUE_NULL, {0}};
    }
    
    int ms = (int)args[arg_idx].as.number;
    #ifdef _WIN32
    Sleep(ms);
    #else
    usleep(ms * 1000);
    #endif
    
    return (Value){VALUE_NULL, {0}};
}

Value stdlib_sys_exit(Value *args, int argc) {
    int arg_idx = 0;
    if (argc > 0 && args[0].type == VALUE_OBJECT) arg_idx = 1;
    
    int exit_code = 0;
    if (argc >= arg_idx + 1 && args[arg_idx].type == VALUE_NUMBER) {
        exit_code = (int)args[arg_idx].as.number;
    }
    
    exit(exit_code);
    return (Value){VALUE_NULL, {0}};
}

/* ASM/JIT Methods */
Value stdlib_asm_alloc_exec(Value *args, int argc) {
    int arg_idx = 0;
    if (argc > 0 && args[0].type == VALUE_OBJECT) arg_idx = 1;
    
    if (argc < arg_idx + 1 || args[arg_idx].type != VALUE_NUMBER) {
        printf("Error: asm.alloc_exec expects size\n");
        return (Value){VALUE_NULL, {0}};
    }
    
    size_t size = (size_t)args[arg_idx].as.number;
    void *ptr = NULL;
    
    #ifdef _WIN32
    ptr = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    #else
    ptr = mmap(NULL, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED) ptr = NULL;
    #endif
    
    if (!ptr) {
        printf("Error: Failed to allocate executable memory\n");
        return (Value){VALUE_NULL, {0}};
    }
    
    Value result;
    result.type = VALUE_FILE; /* Using FILE wrapper for pointer */
    result.as.file = ptr;
    return result;
}

Value stdlib_asm_exec(Value *args, int argc) {
    int arg_idx = 0;
    if (argc > 0 && args[0].type == VALUE_OBJECT) arg_idx = 1;
    
    if (argc < arg_idx + 1 || args[arg_idx].type != VALUE_FILE) {
        printf("Error: asm.exec expects executable pointer\n");
        return (Value){VALUE_NULL, {0}};
    }
    
    typedef int (*JitFunc)(void);
    JitFunc func = (JitFunc)args[arg_idx].as.file;
    
    if (func) {
        int res = func();
        return (Value){VALUE_NUMBER, .as.number = (double)res};
    }
    
    return (Value){VALUE_NULL, {0}};
}
