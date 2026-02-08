#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "parser.h"

typedef enum {
    VALUE_NUMBER,
    VALUE_STRING,
    VALUE_NULL,
    VALUE_BOOLEAN,
    VALUE_LIST,
    VALUE_DICT,
    VALUE_FILE,
    VALUE_FUNCTION,
    VALUE_CLASS,
    VALUE_OBJECT
} ValueType;

typedef struct Value Value;
typedef struct List List;
typedef struct Dict Dict;

struct Value {
    ValueType type;
    union {
        double number;
        char *string;
        int boolean;
        List *list;
        Dict *dict;
        FILE *file;
        ASTNode *function;
        ASTNode *class_def;
    } as;
};

struct List {
    Value *items;
    size_t count;
    size_t capacity;
};

typedef struct DictEntry {
    char *key;
    Value *value;
    struct DictEntry *next;
} DictEntry;

struct Dict {
    DictEntry **buckets;
    size_t bucket_count;
    size_t count;
};

typedef struct {
    char *name;
    Value value;
} Variable;

typedef struct {
    Variable *variables;
    size_t count;
    size_t capacity;
} Scope;

typedef struct {
    Scope *scopes;
    size_t scope_count;
    size_t scope_capacity;
    int in_loop;
    int break_loop;
    int continue_loop;
    int error_occurred;
    char error_message[256];
} Interpreter;

Interpreter* interpreter_create(void);
void interpreter_destroy(Interpreter *interpreter);
void interpreter_push_scope(Interpreter *interpreter);
void interpreter_pop_scope(Interpreter *interpreter);
void interpreter_set_variable(Interpreter *interpreter, const char *name, Value value);
Value interpreter_get_variable(Interpreter *interpreter, const char *name);
Value interpreter_eval(Interpreter *interpreter, ASTNode *node);
Value interpreter_call_function(Interpreter *interpreter, ASTNode *node);

#endif
