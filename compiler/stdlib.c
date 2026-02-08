#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "tess_stdlib.h"

Value stdlib_print(Value *args, int argc) {
    for (int i = 0; i < argc; i++) {
        if (args[i].type == VALUE_STRING) {
            printf("%s", args[i].as.string);
        } else if (args[i].type == VALUE_NUMBER) {
            printf("%g", args[i].as.number);
        } else if (args[i].type == VALUE_NULL) {
            printf("null");
        } else if (args[i].type == VALUE_BOOLEAN) {
            printf(args[i].as.boolean ? "true" : "false");
        } else if (args[i].type == VALUE_LIST) {
            printf("[List]");
        } else if (args[i].type == VALUE_DICT) {
            printf("[Dict]");
        }
        if (i < argc - 1) printf(" ");
    }
    printf("\n");
    return (Value){VALUE_NULL, {0}};
}

Value stdlib_sqrt(Value *args, int argc) {
    if (argc < 1 || args[0].type != VALUE_NUMBER) {
        return (Value){VALUE_NULL, {0}};
    }
    return (Value){VALUE_NUMBER, {.number = sqrt(args[0].as.number)}};
}

Value stdlib_len(Value *args, int argc) {
    if (argc < 1) return (Value){VALUE_NUMBER, {.number = 0}};
    
    if (args[0].type == VALUE_STRING) {
        return (Value){VALUE_NUMBER, {.number = strlen(args[0].as.string)}};
    } else if (args[0].type == VALUE_LIST) {
        return (Value){VALUE_NUMBER, {.number = args[0].as.list->count}};
    } else if (args[0].type == VALUE_DICT) {
        return (Value){VALUE_NUMBER, {.number = args[0].as.dict->count}};
    }
    
    return (Value){VALUE_NUMBER, {.number = 0}};
}

Value stdlib_file_open(Value *args, int argc) {
    if (argc < 2 || args[0].type != VALUE_STRING || args[1].type != VALUE_STRING) {
        return (Value){VALUE_NULL, {0}};
    }
    
    FILE *f = fopen(args[0].as.string, args[1].as.string);
    if (!f) return (Value){VALUE_NULL, {0}};
    
    Value val;
    val.type = VALUE_FILE;
    val.as.file = f;
    return val;
}

Value stdlib_file_write(Value *args, int argc) {
    if (argc < 2 || args[0].type != VALUE_FILE || args[1].type != VALUE_STRING) {
        return (Value){VALUE_NULL, {0}};
    }
    
    fputs(args[1].as.string, args[0].as.file);
    return (Value){VALUE_NULL, {0}};
}

Value stdlib_file_read(Value *args, int argc) {
    if (argc < 1 || args[0].type != VALUE_FILE) {
        return (Value){VALUE_NULL, {0}};
    }
    
    FILE *f = args[0].as.file;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char *content = malloc(size + 1);
    fread(content, 1, size, f);
    content[size] = '\0';
    
    Value val;
    val.type = VALUE_STRING;
    val.as.string = content;
    return val;
}

Value stdlib_file_close(Value *args, int argc) {
    if (argc < 1 || args[0].type != VALUE_FILE) {
        return (Value){VALUE_NULL, {0}};
    }
    
    fclose(args[0].as.file);
    return (Value){VALUE_NULL, {0}};
}

Value stdlib_read_file(Value *args, int argc) {
    if (argc < 1 || args[0].type != VALUE_STRING) {
        return (Value){VALUE_NULL, {0}};
    }
    
    FILE *f = fopen(args[0].as.string, "r");
    if (!f) return (Value){VALUE_NULL, {0}};
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char *content = malloc(size + 1);
    fread(content, 1, size, f);
    content[size] = '\0';
    fclose(f);
    
    Value val;
    val.type = VALUE_STRING;
    val.as.string = content;
    return val;
}

Value stdlib_write_file(Value *args, int argc) {
    if (argc < 2 || args[0].type != VALUE_STRING || args[1].type != VALUE_STRING) {
        return (Value){VALUE_NUMBER, {.number = 0}};
    }
    
    FILE *f = fopen(args[0].as.string, "w");
    if (!f) return (Value){VALUE_NUMBER, {.number = 0}};
    
    fputs(args[1].as.string, f);
    fclose(f);
    
    return (Value){VALUE_NUMBER, {.number = 1}};
}

Value stdlib_mem_alloc(Value *args, int argc) {
    if (argc < 1 || args[0].type != VALUE_NUMBER) {
        return (Value){VALUE_NULL, {0}};
    }
    
    size_t size = (size_t)args[0].as.number;
    void *ptr = malloc(size);
    
    Value val;
    val.type = VALUE_NUMBER;
    val.as.number = (double)(unsigned long long)ptr;
    return val;
}

Value stdlib_mem_free(Value *args, int argc) {
    if (argc < 1 || args[0].type != VALUE_NUMBER) {
        return (Value){VALUE_NULL, {0}};
    }
    
    void *ptr = (void*)(unsigned long long)args[0].as.number;
    free(ptr);
    return (Value){VALUE_NULL, {0}};
}

Value stdlib_mem_write(Value *args, int argc) {
    if (argc < 3 || args[0].type != VALUE_NUMBER || args[1].type != VALUE_NUMBER || args[2].type != VALUE_NUMBER) {
        return (Value){VALUE_NULL, {0}};
    }
    
    void *ptr = (void*)(unsigned long long)args[0].as.number;
    size_t offset = (size_t)args[1].as.number;
    unsigned char val = (unsigned char)args[2].as.number;
    
    ((unsigned char*)ptr)[offset] = val;
    return (Value){VALUE_NULL, {0}};
}

Value stdlib_mem_read(Value *args, int argc) {
    if (argc < 2 || args[0].type != VALUE_NUMBER || args[1].type != VALUE_NUMBER) {
        return (Value){VALUE_NULL, {0}};
    }
    
    void *ptr = (void*)(unsigned long long)args[0].as.number;
    size_t offset = (size_t)args[1].as.number;
    
    unsigned char val = ((unsigned char*)ptr)[offset];
    
    Value res;
    res.type = VALUE_NUMBER;
    res.as.number = val;
    return res;
}

Value stdlib_sys_sleep(Value *args, int argc) {
    if (argc < 1 || args[0].type != VALUE_NUMBER) {
        return (Value){VALUE_NULL, {0}};
    }
    
#ifdef _WIN32
    _sleep((int)args[0].as.number);
#else
    usleep((int)(args[0].as.number * 1000));
#endif
    return (Value){VALUE_NULL, {0}};
}

Value stdlib_sys_exit(Value *args, int argc) {
    int code = 0;
    if (argc > 0 && args[0].type == VALUE_NUMBER) {
        code = (int)args[0].as.number;
    }
    exit(code);
    return (Value){VALUE_NULL, {0}};
}

Value stdlib_asm_alloc_exec(Value *args, int argc) {
    if (argc < 1 || args[0].type != VALUE_NUMBER) {
        return (Value){VALUE_NULL, {0}};
    }
    
    size_t size = (size_t)args[0].as.number;
    
#ifdef _WIN32
    void *ptr = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
#else
    void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#endif
    
    Value val;
    val.type = VALUE_NUMBER;
    val.as.number = (double)(unsigned long long)ptr;
    return val;
}

Value stdlib_asm_exec(Value *args, int argc) {
    if (argc < 1 || args[0].type != VALUE_NUMBER) {
        return (Value){VALUE_NULL, {0}};
    }
    
    void (*func)() = (void (*)())(unsigned long long)args[0].as.number;
    func();
    
    return (Value){VALUE_NULL, {0}};
}

Value stdlib_json_format(Value *args, int argc) {
    if (argc < 1) return (Value){VALUE_NULL, {0}};
    
    Value v = args[0];
    if (v.type == VALUE_STRING) {
        size_t len = strlen(v.as.string) + 3;
        char *s = malloc(len);
        snprintf(s, len, "\"%s\"", v.as.string);
        return (Value){VALUE_STRING, {.string = s}};
    } else if (v.type == VALUE_NUMBER) {
        char *s = malloc(32);
        snprintf(s, 32, "%g", v.as.number);
        return (Value){VALUE_STRING, {.string = s}};
    } else if (v.type == VALUE_NULL) {
        return (Value){VALUE_STRING, {.string = strdup("null")}};
    } else if (v.type == VALUE_BOOLEAN) {
        return (Value){VALUE_STRING, {.string = strdup(v.as.boolean ? "true" : "false")}};
    }
    
    return (Value){VALUE_STRING, {.string = strdup("{}")}}; 
}

Value stdlib_clock(Value *args, int argc) {
    (void)args;
    (void)argc;
    Value v;
    v.type = VALUE_NUMBER;
    v.as.number = (double)clock() / CLOCKS_PER_SEC;
    return v;
}
