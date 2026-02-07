#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "parser.h"
#include <stdlib.h>

/* Forward declaration of List and Dict to resolve circular dependency */
typedef struct Value Value;

typedef struct {
    Value *items;
    size_t count;
    size_t capacity;
} List;

typedef struct DictEntry {
    char *key;
    Value *value;
    struct DictEntry *next;
} DictEntry;

typedef struct {
    DictEntry **buckets;
    size_t bucket_count;
    size_t count;
} Dict;

typedef enum {
    VALUE_NULL,
    VALUE_NUMBER,
    VALUE_STRING,
    VALUE_BOOLEAN,
    VALUE_FUNCTION,
    VALUE_CLASS,    /* Class definition */
    VALUE_OBJECT,   /* Instance (implemented as Dict) */
    VALUE_FILE,     /* Native File Handle */
    VALUE_LIST,
    VALUE_DICT
} ValueType;

struct Value {
    ValueType type;
    union {
        double number;
        char *string;
        int boolean;
        ASTNode *function;
        ASTNode *class_def; /* Pointer to AST_CLASS_DEF node */
        Dict *dict;         /* For both DICT and OBJECT types */
        List *list;
        void *file;         /* FILE* pointer */
    } as;
};

typedef struct Variable {
    char *name;
    Value value;
} Variable;

typedef struct Scope {
    Variable *variables;
    size_t count;
    size_t capacity;
} Scope;

typedef struct {
    Scope *scopes;
    size_t scope_count;
    size_t scope_capacity;
    int in_loop;        /* Loop nesting level */
    int break_loop;     /* Flag to break out of loop */
    int continue_loop;  /* Flag to continue loop */
    int error_occurred;
    char error_message[256];
} Interpreter;

Interpreter* interpreter_create(void);
void interpreter_destroy(Interpreter *interpreter);
Value interpreter_eval(Interpreter *interpreter, ASTNode *node);
Value interpreter_call_function(Interpreter *interpreter, ASTNode *node);
void interpreter_push_scope(Interpreter *interpreter);
void interpreter_pop_scope(Interpreter *interpreter);
void interpreter_set_variable(Interpreter *interpreter, const char *name, Value value);
Value interpreter_get_variable(Interpreter *interpreter, const char *name);

#endif /* INTERPRETER_H */
