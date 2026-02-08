#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#ifdef _WIN32
#include <io.h>
#include <direct.h>
#define access _access
#define F_OK 0
#else
#include <unistd.h>
#endif
#include "interpreter.h"
#include "tess_stdlib.h"

typedef Value (*BuiltinFunc)(Value *args, int argc);

typedef struct {
    char *name;
    BuiltinFunc func;
} BuiltinFunction;

static Value str_len(Value *args, int argc);
static Value str_slice(Value *args, int argc);
static Value str_replace(Value *args, int argc);
static Value list_append(Value *args, int argc);
static Value list_pop(Value *args, int argc);
static Value dict_keys(Value *args, int argc);
static Value dict_values(Value *args, int argc);
static Value math_abs(Value *args, int argc);
static Value math_max(Value *args, int argc);
static Value math_min(Value *args, int argc);
static Value get_timing(Value *args, int argc);

static BuiltinFunction builtins[] = {
    {"read_file", (BuiltinFunc)stdlib_read_file},
    {"write_file", (BuiltinFunc)stdlib_write_file},
    {"sqrt", (BuiltinFunc)stdlib_sqrt},
    {"len", (BuiltinFunc)stdlib_len},
    {"print", (BuiltinFunc)stdlib_print},
    {"abs", (BuiltinFunc)math_abs},
    {"max", (BuiltinFunc)math_max},
    {"min", (BuiltinFunc)math_min},
    {"str_len", (BuiltinFunc)str_len},
    {"str_slice", (BuiltinFunc)str_slice},
    {"str_replace", (BuiltinFunc)str_replace},
    {"list_append", (BuiltinFunc)list_append},
    {"list_pop", (BuiltinFunc)list_pop},
    {"dict_keys", (BuiltinFunc)dict_keys},
    {"dict_values", (BuiltinFunc)dict_values},
    {"json_format", (BuiltinFunc)stdlib_json_format},
    {"clock", (BuiltinFunc)stdlib_clock},
    {"timing", (BuiltinFunc)get_timing},
    {NULL, NULL}
};

static Value str_len(Value *args, int argc) {
    Value result = {VALUE_NUMBER, {0}};
    if (argc < 1 || args[0].type != VALUE_STRING) {
        return result;
    }
    result.as.number = strlen(args[0].as.string);
    return result;
}

static Value str_slice(Value *args, int argc) {
    Value result = {VALUE_STRING, {0}};
    if (argc < 3 || args[0].type != VALUE_STRING) {
        result.type = VALUE_NULL;
        return result;
    }
    char *str = args[0].as.string;
    int start = (int)args[1].as.number;
    int end = (int)args[2].as.number;
    int len = strlen(str);
    
    if (start < 0) start = len + start;
    if (end < 0) end = len + end;
    if (start < 0) start = 0;
    if (end > len) end = len;
    if (start > end) {
        result.as.string = strdup("");
        return result;
    }
    
    int slice_len = end - start;
    result.as.string = malloc(slice_len + 1);
    memcpy(result.as.string, str + start, slice_len);
    result.as.string[slice_len] = '\0';
    return result;
}

static Value str_replace(Value *args, int argc) {
    Value result = {VALUE_STRING, {0}};
    if (argc < 3 || args[0].type != VALUE_STRING || args[1].type != VALUE_STRING || args[2].type != VALUE_STRING) {
        result.type = VALUE_NULL;
        return result;
    }
    char *str = args[0].as.string;
    char *old = args[1].as.string;
    char *new = args[2].as.string;
    
    size_t result_len = strlen(str) + 1;
    char *result_str = malloc(result_len);
    strcpy(result_str, str);
    
    char *pos = strstr(result_str, old);
    if (pos) {
        size_t old_len = strlen(old);
        size_t new_len = strlen(new);
        size_t tail_len = strlen(pos + old_len);
        result_len = (pos - result_str) + new_len + tail_len + 1;
        char *new_str = malloc(result_len);
        memcpy(new_str, result_str, pos - result_str);
        memcpy(new_str + (pos - result_str), new, new_len);
        strcpy(new_str + (pos - result_str) + new_len, pos + old_len);
        free(result_str);
        result_str = new_str;
    }
    result.as.string = result_str;
    return result;
}

static Value list_append(Value *args, int argc) {
    Value result = {VALUE_NULL, {0}};
    if (argc < 2 || args[0].type != VALUE_LIST) {
        return result;
    }
    List *list = args[0].as.list;
    if (list->count >= list->capacity) {
        list->capacity *= 2;
        list->items = realloc(list->items, sizeof(Value) * list->capacity);
    }
    list->items[list->count++] = args[1];
    result.type = VALUE_NUMBER;
    result.as.number = 1;
    return result;
}

static Value list_pop(Value *args, int argc) {
    Value result = {VALUE_NULL, {0}};
    if (argc < 1 || args[0].type != VALUE_LIST) {
        return result;
    }
    List *list = args[0].as.list;
    if (list->count > 0) {
        result = list->items[--list->count];
    }
    return result;
}

static Value dict_keys(Value *args, int argc) {
    Value result = {VALUE_LIST, {0}};
    if (argc < 1 || args[0].type != VALUE_DICT) {
        result.type = VALUE_NULL;
        return result;
    }
    Dict *dict = args[0].as.dict;
    List *list = malloc(sizeof(List));
    list->count = 0;
    list->capacity = dict->count;
    list->items = malloc(sizeof(Value) * list->capacity);
    
    for (size_t i = 0; i < dict->bucket_count; i++) {
        DictEntry *entry = dict->buckets[i];
        while (entry) {
            Value key_val = {VALUE_STRING, {0}};
            key_val.as.string = strdup(entry->key);
            list->items[list->count++] = key_val;
            entry = entry->next;
        }
    }
    result.as.list = list;
    return result;
}

static Value dict_values(Value *args, int argc) {
    Value result = {VALUE_LIST, {0}};
    if (argc < 1 || args[0].type != VALUE_DICT) {
        result.type = VALUE_NULL;
        return result;
    }
    Dict *dict = args[0].as.dict;
    List *list = malloc(sizeof(List));
    list->count = 0;
    list->capacity = dict->count;
    list->items = malloc(sizeof(Value) * list->capacity);
    
    for (size_t i = 0; i < dict->bucket_count; i++) {
        DictEntry *entry = dict->buckets[i];
        while (entry) {
            list->items[list->count++] = *entry->value;
            entry = entry->next;
        }
    }
    result.as.list = list;
    return result;
}

static Value math_abs(Value *args, int argc) {
    Value result = {VALUE_NUMBER, {0}};
    if (argc < 1 || args[0].type != VALUE_NUMBER) {
        return result;
    }
    result.as.number = fabs(args[0].as.number);
    return result;
}

static Value math_max(Value *args, int argc) {
    Value result = {VALUE_NUMBER, {0}};
    if (argc < 1) return result;
    double max_val = args[0].type == VALUE_NUMBER ? args[0].as.number : 0;
    for (int i = 1; i < argc; i++) {
        if (args[i].type == VALUE_NUMBER && args[i].as.number > max_val) {
            max_val = args[i].as.number;
        }
    }
    result.as.number = max_val;
    return result;
}

static Value math_min(Value *args, int argc) {
    Value result = {VALUE_NUMBER, {0}};
    if (argc < 1) return result;
    double min_val = args[0].type == VALUE_NUMBER ? args[0].as.number : 0;
    for (int i = 1; i < argc; i++) {
        if (args[i].type == VALUE_NUMBER && args[i].as.number < min_val) {
            min_val = args[i].as.number;
        }
    }
    result.as.number = min_val;
    return result;
}

static Value get_timing(Value *args, int argc) {
    (void)args;
    (void)argc;
    
    extern double g_compile_time, g_execute_time, g_total_time;
    
    Value result = {VALUE_DICT, {0}};
    Dict *dict = malloc(sizeof(Dict));
    dict->bucket_count = 8;
    dict->count = 0;
    dict->buckets = calloc(dict->bucket_count, sizeof(DictEntry*));
    
    Value compile_val = {VALUE_NUMBER, {0}};
    compile_val.as.number = g_compile_time;
    DictEntry *compile_entry = malloc(sizeof(DictEntry));
    compile_entry->key = strdup("compile_time");
    compile_entry->value = malloc(sizeof(Value));
    *compile_entry->value = compile_val;
    compile_entry->next = NULL;
    unsigned long hash = 5381;
    char *k = compile_entry->key;
    int c;
    while ((c = *k++)) hash = ((hash << 5) + hash) + c;
    size_t bucket_idx = hash % dict->bucket_count;
    compile_entry->next = dict->buckets[bucket_idx];
    dict->buckets[bucket_idx] = compile_entry;
    dict->count++;
    
    Value execute_val = {VALUE_NUMBER, {0}};
    execute_val.as.number = g_execute_time;
    DictEntry *execute_entry = malloc(sizeof(DictEntry));
    execute_entry->key = strdup("execute_time");
    execute_entry->value = malloc(sizeof(Value));
    *execute_entry->value = execute_val;
    execute_entry->next = NULL;
    hash = 5381;
    k = execute_entry->key;
    while ((c = *k++)) hash = ((hash << 5) + hash) + c;
    bucket_idx = hash % dict->bucket_count;
    execute_entry->next = dict->buckets[bucket_idx];
    dict->buckets[bucket_idx] = execute_entry;
    dict->count++;
    
    Value total_val = {VALUE_NUMBER, {0}};
    total_val.as.number = g_total_time;
    DictEntry *total_entry = malloc(sizeof(DictEntry));
    total_entry->key = strdup("total_time");
    total_entry->value = malloc(sizeof(Value));
    *total_entry->value = total_val;
    total_entry->next = NULL;
    hash = 5381;
    k = total_entry->key;
    while ((c = *k++)) hash = ((hash << 5) + hash) + c;
    bucket_idx = hash % dict->bucket_count;
    total_entry->next = dict->buckets[bucket_idx];
    dict->buckets[bucket_idx] = total_entry;
    dict->count++;
    
    result.as.dict = dict;
    return result;
}

BuiltinFunc get_builtin(const char *name) {
    for (int i = 0; builtins[i].name; i++) {
        if (strcmp(builtins[i].name, name) == 0) {
            return builtins[i].func;
        }
    }
    
    if (strcmp(name, "len") == 0) return (BuiltinFunc)str_len;
    if (strcmp(name, "slice") == 0) return (BuiltinFunc)str_slice;
    if (strcmp(name, "replace") == 0) return (BuiltinFunc)str_replace;
    
    if (strcmp(name, "append") == 0) return (BuiltinFunc)list_append;
    if (strcmp(name, "pop") == 0) return (BuiltinFunc)list_pop;
    
    if (strcmp(name, "keys") == 0) return (BuiltinFunc)dict_keys;
    if (strcmp(name, "values") == 0) return (BuiltinFunc)dict_values;
    
    if (strcmp(name, "abs") == 0) return (BuiltinFunc)math_abs;
    if (strcmp(name, "max") == 0) return (BuiltinFunc)math_max;
    if (strcmp(name, "min") == 0) return (BuiltinFunc)math_min;
    
    if (strcmp(name, "timing") == 0) return (BuiltinFunc)get_timing;
    
    if (strcmp(name, "open") == 0) return (BuiltinFunc)stdlib_file_open;
    if (strcmp(name, "write") == 0) return (BuiltinFunc)stdlib_file_write;
    if (strcmp(name, "read") == 0) return (BuiltinFunc)stdlib_file_read;
    if (strcmp(name, "close") == 0) return (BuiltinFunc)stdlib_file_close;

    if (strcmp(name, "alloc") == 0) return (BuiltinFunc)stdlib_mem_alloc;
    if (strcmp(name, "free") == 0) return (BuiltinFunc)stdlib_mem_free;
    if (strcmp(name, "set") == 0) return (BuiltinFunc)stdlib_mem_write;
    if (strcmp(name, "get") == 0) return (BuiltinFunc)stdlib_mem_read;
    
    if (strcmp(name, "sleep") == 0) return (BuiltinFunc)stdlib_sys_sleep;
    if (strcmp(name, "exit") == 0) return (BuiltinFunc)stdlib_sys_exit;
    
    if (strcmp(name, "alloc_exec") == 0) return (BuiltinFunc)stdlib_asm_alloc_exec;
    if (strcmp(name, "exec") == 0) return (BuiltinFunc)stdlib_asm_exec;

    return NULL;
}

void register_builtins(Interpreter *interpreter) {
    for (int i = 0; builtins[i].name; i++) {
        Value func_val = {VALUE_FUNCTION, {0}};
        func_val.as.function = NULL;
        interpreter_set_variable(interpreter, builtins[i].name, func_val);
    }
    
    Value f_obj = {VALUE_OBJECT, {0}};
    Dict *f_dict = malloc(sizeof(Dict));
    f_dict->bucket_count = 8;
    f_dict->count = 0;
    f_dict->buckets = calloc(8, sizeof(DictEntry*));
    
    Value open_func = {VALUE_FUNCTION, {0}};
    open_func.as.function = NULL;
    
    DictEntry *entry = malloc(sizeof(DictEntry));
    entry->key = strdup("open");
    entry->value = malloc(sizeof(Value));
    *entry->value = open_func;
    entry->next = NULL;
    
    unsigned long hash = 5381;
    char *k = "open";
    int c;
    while ((c = *k++)) hash = ((hash << 5) + hash) + c;
    size_t idx = hash % 8;
    f_dict->buckets[idx] = entry;
    
    f_obj.as.dict = f_dict;
    interpreter_set_variable(interpreter, "f", f_obj);

    Value mem_obj = {VALUE_OBJECT, {0}};
    Dict *mem_dict = malloc(sizeof(Dict));
    mem_dict->bucket_count = 8;
    mem_dict->count = 0;
    mem_dict->buckets = calloc(8, sizeof(DictEntry*));

    void add_mem_method(const char *key, Dict *target_dict) {
        Value func = {VALUE_FUNCTION, {0}};
        func.as.function = NULL;
        
        DictEntry *e = malloc(sizeof(DictEntry));
        e->key = strdup(key);
        e->value = malloc(sizeof(Value));
        *e->value = func;
        e->next = NULL;
        
        unsigned long h = 5381;
        const char *str = key;
        int ch;
        while ((ch = *str++)) h = ((h << 5) + h) + ch;
        size_t i = h % 8;
        
        e->next = target_dict->buckets[i];
        target_dict->buckets[i] = e;
        target_dict->count++;
    }

    add_mem_method("alloc", mem_dict);
    add_mem_method("free", mem_dict);
    add_mem_method("set", mem_dict);
    add_mem_method("get", mem_dict);

    mem_obj.as.dict = mem_dict;
    interpreter_set_variable(interpreter, "mem", mem_obj);
    
    Value sys_obj = {VALUE_OBJECT, {0}};
    Dict *sys_dict = malloc(sizeof(Dict));
    sys_dict->bucket_count = 8;
    sys_dict->count = 0;
    sys_dict->buckets = calloc(8, sizeof(DictEntry*));
    
    add_mem_method("sleep", sys_dict);
    add_mem_method("exit", sys_dict);
    
    sys_obj.as.dict = sys_dict;
    interpreter_set_variable(interpreter, "sys", sys_obj);
    
    Value asm_obj = {VALUE_OBJECT, {0}};
    Dict *asm_dict = malloc(sizeof(Dict));
    asm_dict->bucket_count = 8;
    asm_dict->count = 0;
    asm_dict->buckets = calloc(8, sizeof(DictEntry*));
    
    add_mem_method("alloc_exec", asm_dict);
    add_mem_method("exec", asm_dict);
    
    asm_obj.as.dict = asm_dict;
    interpreter_set_variable(interpreter, "asm", asm_obj);
}
