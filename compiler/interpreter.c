#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "interpreter.h"
#include "module.h"
#include "tess_stdlib.h"
#include "builtins.h"
#include "http_client.h"

static char *error_message = NULL;

Interpreter* interpreter_create(void) {
    Interpreter *interpreter = malloc(sizeof(Interpreter));
    interpreter->scopes = NULL;
    interpreter->scope_count = 0;
    interpreter->scope_capacity = 0;
    interpreter->in_loop = 0;
    interpreter->break_loop = 0;
    interpreter->continue_loop = 0;
    interpreter->error_occurred = 0;
    memset(interpreter->error_message, 0, sizeof(interpreter->error_message));
    
    interpreter_push_scope(interpreter);
    register_builtins(interpreter);
    
    return interpreter;
}

void interpreter_destroy(Interpreter *interpreter) {
    if (interpreter) {
        if (interpreter->scopes) {
            for (size_t i = 0; i < interpreter->scope_count; i++) {
                for (size_t j = 0; j < interpreter->scopes[i].count; j++) {
                    free(interpreter->scopes[i].variables[j].name);
                }
                if (interpreter->scopes[i].variables) {
                    free(interpreter->scopes[i].variables);
                }
            }
            free(interpreter->scopes);
        }
        free(interpreter);
    }
}

void interpreter_push_scope(Interpreter *interpreter) {
    if (interpreter->scope_count >= interpreter->scope_capacity) {
        size_t new_capacity = interpreter->scope_capacity == 0 ? 4 : interpreter->scope_capacity * 2;
        interpreter->scopes = realloc(interpreter->scopes, 
                                    sizeof(Scope) * new_capacity);
        interpreter->scope_capacity = new_capacity;
    }
    
    Scope *scope = &interpreter->scopes[interpreter->scope_count++];
    scope->variables = NULL;
    scope->count = 0;
    scope->capacity = 0;
}

void interpreter_pop_scope(Interpreter *interpreter) {
    if (interpreter->scope_count > 0) {
        interpreter->scope_count--;
        Scope *scope = &interpreter->scopes[interpreter->scope_count];
        for (size_t i = 0; i < scope->count; i++) {
            free(scope->variables[i].name);
        }
        if (scope->variables) {
            free(scope->variables);
        }
    }
}

void interpreter_set_variable(Interpreter *interpreter, const char *name, Value value) {
    if (interpreter->scope_count > 0) {
        Scope *scope = &interpreter->scopes[interpreter->scope_count - 1];
        
        for (size_t i = 0; i < scope->count; i++) {
            if (strcmp(scope->variables[i].name, name) == 0) {
                scope->variables[i].value = value;
                return;
            }
        }
        
        if (scope->count >= scope->capacity) {
            size_t new_capacity = scope->capacity == 0 ? 4 : scope->capacity * 2;
            scope->variables = realloc(scope->variables,
                                     sizeof(Variable) * new_capacity);
            scope->capacity = new_capacity;
        }
        
        scope->variables[scope->count].name = strdup(name);
        scope->variables[scope->count].value = value;
        scope->count++;
    }
}

Value interpreter_get_variable(Interpreter *interpreter, const char *name) {
    for (int i = interpreter->scope_count - 1; i >= 0; i--) {
        Scope *scope = &interpreter->scopes[i];
        for (size_t j = 0; j < scope->count; j++) {
            if (strcmp(scope->variables[j].name, name) == 0) {
                return scope->variables[j].value;
            }
        }
    }
    
    Value val = {VALUE_NULL, {0}};
    return val;
}

Value interpreter_eval(Interpreter *interpreter, ASTNode *node) {
    if (!node) {
        Value val = {VALUE_NULL, {0}};
        return val;
    }
    
    if (interpreter->error_occurred && node->type != AST_CATCH) {
        return (Value){VALUE_NULL, {0}};
    }

    switch (node->type) {
        case AST_PROGRAM: {
            ASTNode *stmt = node->children;
            Value result = {VALUE_NULL, {0}};
            while (stmt) {
                result = interpreter_eval(interpreter, stmt);
                stmt = stmt->next;
            }
            return result;
        }

        case AST_NUMBER: {
            Value val;
            val.type = VALUE_NUMBER;
            val.as.number = strtod(node->value, NULL);
            return val;
        }
        
        case AST_STRING: {
            Value val;
            val.type = VALUE_STRING;
            val.as.string = strdup(node->value);
            return val;
        }
        
        case AST_BINARY_OP: {
            Value left = interpreter_eval(interpreter, node->left);
            Value right = interpreter_eval(interpreter, node->right);
            
            if (left.type == VALUE_NUMBER && right.type == VALUE_NUMBER) {
                Value result;
                result.type = VALUE_NUMBER;
                if (strcmp(node->value, "+") == 0) result.as.number = left.as.number + right.as.number;
                else if (strcmp(node->value, "-") == 0) result.as.number = left.as.number - right.as.number;
                else if (strcmp(node->value, "*") == 0) result.as.number = left.as.number * right.as.number;
                else if (strcmp(node->value, "/") == 0) result.as.number = left.as.number / right.as.number;
                else if (strcmp(node->value, "%") == 0) result.as.number = fmod(left.as.number, right.as.number);
                else if (strcmp(node->value, ">") == 0) result.as.number = left.as.number > right.as.number;
                else if (strcmp(node->value, "<") == 0) result.as.number = left.as.number < right.as.number;
                else if (strcmp(node->value, "==") == 0) result.as.number = left.as.number == right.as.number;
                else if (strcmp(node->value, "<=") == 0) result.as.number = left.as.number <= right.as.number;
                else if (strcmp(node->value, ">=") == 0) result.as.number = left.as.number >= right.as.number;
                return result;
            }
            
            if (strcmp(node->value, "+") == 0) {
                char *left_str = NULL;
                char *right_str = NULL;
                
                if (left.type == VALUE_STRING) left_str = left.as.string;
                else if (left.type == VALUE_NUMBER) {
                    left_str = malloc(64);
                    snprintf(left_str, 64, "%g", left.as.number);
                }
                
                if (right.type == VALUE_STRING) right_str = right.as.string;
                else if (right.type == VALUE_NUMBER) {
                    right_str = malloc(64);
                    snprintf(right_str, 64, "%g", right.as.number);
                }
                
                if (left_str && right_str) {
                    size_t len = strlen(left_str) + strlen(right_str) + 1;
                    Value result;
                    result.type = VALUE_STRING;
                    result.as.string = malloc(len);
                    snprintf(result.as.string, len, "%s%s", left_str, right_str);
                    
                    if (left.type != VALUE_STRING && left_str) free(left_str);
                    if (right.type != VALUE_STRING && right_str) free(right_str);
                    
                    return result;
                }
            }
            break;
        }
        
        case AST_ASSIGNMENT: {
            Value val = interpreter_eval(interpreter, node->right);
            interpreter_set_variable(interpreter, node->value, val);
            return val;
        }
        
        case AST_IDENTIFIER: {
            return interpreter_get_variable(interpreter, node->value);
        }
        
        case AST_PRINT: {
            ASTNode *expr = node->left;
            Value last_val = {VALUE_NULL, {0}};
            
            while (expr) {
                Value val = interpreter_eval(interpreter, expr);
                if (val.type == VALUE_NUMBER) {
                    printf("%g", val.as.number);
                } else if (val.type == VALUE_STRING) {
                    printf("%s", val.as.string);
                } else if (val.type == VALUE_NULL) {
                    printf("null");
                } else if (val.type == VALUE_BOOLEAN) {
                    printf(val.as.boolean ? "true" : "false");
                } else if (val.type == VALUE_LIST) {
                    printf("[List]");
                } else if (val.type == VALUE_DICT) {
                    printf("[Dict]");
                } else if (val.type == VALUE_FILE) {
                    printf("[File Handle: %p]", val.as.file);
                } else if (val.type == VALUE_FUNCTION) {
                    printf("[Function]");
                } else if (val.type == VALUE_CLASS) {
                    printf("[Class]");
                } else if (val.type == VALUE_OBJECT) {
                    printf("[Object]");
                }
                
                if (expr->next) printf(" ");
                last_val = val;
                expr = expr->next;
            }
            printf("\n");
            fflush(stdout);
            return last_val;
        }
        
        case AST_BLOCK: {
            interpreter_push_scope(interpreter);
            ASTNode *stmt = node->children;
            Value result = {VALUE_NULL, {0}};
            while (stmt) {
                result = interpreter_eval(interpreter, stmt);
                Value ret_check = interpreter_get_variable(interpreter, "__return__");
                if (ret_check.type == VALUE_NUMBER && ret_check.as.number == 1) {
                    result = interpreter_get_variable(interpreter, "__return_value__");
                    interpreter_set_variable(interpreter, "__return__", (Value){VALUE_NULL, {0}});
                    interpreter_pop_scope(interpreter);
                    return result;
                }
                if (interpreter->break_loop || interpreter->continue_loop) {
                    break;
                }
                stmt = stmt->next;
            }
            interpreter_pop_scope(interpreter);
            return result;
        }
        
        case AST_REPEAT: {
            ASTNode *count_node = node->left;
            ASTNode *block_node = node->children;
            Value count_val = interpreter_eval(interpreter, count_node);
            
            if (count_val.type == VALUE_NUMBER) {
                int count = (int)count_val.as.number;
                interpreter->in_loop++;
                for (int i = 0; i < count; i++) {
                    interpreter_eval(interpreter, block_node);
                    
                    if (interpreter->break_loop) {
                        interpreter->break_loop = 0;
                        break;
                    }
                    if (interpreter->continue_loop) {
                        interpreter->continue_loop = 0;
                        continue;
                    }
                }
                interpreter->in_loop--;
            }
            break;
        }
        
        case AST_WHILE: {
            ASTNode *condition = node->left;
            ASTNode *block = node->children;
            
            interpreter->in_loop++;
            while (1) {
                Value cond_val = interpreter_eval(interpreter, condition);
                if (cond_val.type == VALUE_NUMBER && cond_val.as.number == 0) break;
                if (cond_val.type == VALUE_NULL) break;
                if (cond_val.type == VALUE_BOOLEAN && !cond_val.as.boolean) break;
                
                interpreter_eval(interpreter, block);
                
                if (interpreter->break_loop) {
                    interpreter->break_loop = 0;
                    break;
                }
                if (interpreter->continue_loop) {
                    interpreter->continue_loop = 0;
                    continue;
                }
            }
            interpreter->in_loop--;
            break;
        }
        
        case AST_BREAK:
            if (interpreter->in_loop) {
                interpreter->break_loop = 1;
            }
            break;
            
        case AST_CONTINUE:
            if (interpreter->in_loop) {
                interpreter->continue_loop = 1;
            }
            break;
            
        case AST_RETURN: {
            Value ret_val = interpreter_eval(interpreter, node->left);
            Value return_wrapper;
            return_wrapper.type = VALUE_NUMBER;
            return_wrapper.as.number = 1;
            interpreter_set_variable(interpreter, "__return__", return_wrapper);
            interpreter_set_variable(interpreter, "__return_value__", ret_val);
            return ret_val;
        }
            
        case AST_TRY: {
            ASTNode *try_block = node->left;
            ASTNode *catch_block = node->right;
            
            interpreter->error_occurred = 0;
            Value result = interpreter_eval(interpreter, try_block);
            
            if (interpreter->error_occurred) {
                interpreter->error_occurred = 0;
                interpreter_push_scope(interpreter);
                if (error_message) {
                    Value err_val;
                    err_val.type = VALUE_STRING;
                    err_val.as.string = strdup(error_message);
                    interpreter_set_variable(interpreter, "error", err_val);
                    free(error_message);
                    error_message = NULL;
                }
                Value catch_res = interpreter_eval(interpreter, catch_block);
                interpreter_pop_scope(interpreter);
                return catch_res;
            }
            return result;
        }
        
        case AST_NEW: {
            char *class_name = node->value;
            Value class_val = interpreter_get_variable(interpreter, class_name);
            
            if (class_val.type == VALUE_CLASS) {
                Value obj;
                obj.type = VALUE_OBJECT;
                obj.as.dict = malloc(sizeof(Dict));
                Dict *instance = obj.as.dict;
                instance->bucket_count = 16;
                instance->buckets = calloc(instance->bucket_count, sizeof(DictEntry*));
                instance->count = 0;
                
                ASTNode *class_def = class_val.as.class_def;
                ASTNode *member = class_def->children;
                while (member) {
                    if (member->type == AST_FUNCTION_DEF) {
                        Value method_val;
                        method_val.type = VALUE_FUNCTION;
                        method_val.as.function = member;
                        
                        unsigned long hash = 5381;
                        int c;
                        char *str = member->value;
                        while ((c = *str++)) hash = ((hash << 5) + hash) + c;
                        size_t index = hash % instance->bucket_count;
                        
                        DictEntry *entry = malloc(sizeof(DictEntry));
                        entry->key = strdup(member->value);
                        entry->value = malloc(sizeof(Value));
                        *entry->value = method_val;
                        entry->next = instance->buckets[index];
                        instance->buckets[index] = entry;
                        instance->count++;
                    }
                    member = member->next;
                }
                
                return obj;
            }
            break;
        }
        
        case AST_LIST: {
            Value list_val;
            list_val.type = VALUE_LIST;
            list_val.as.list = malloc(sizeof(List));
            List *list = list_val.as.list;
            list->count = 0;
            list->capacity = 4;
            list->items = malloc(sizeof(Value) * list->capacity);
            
            ASTNode *item = node->children;
            while (item) {
                if (list->count >= list->capacity) {
                    list->capacity *= 2;
                    list->items = realloc(list->items, sizeof(Value) * list->capacity);
                }
                list->items[list->count++] = interpreter_eval(interpreter, item);
                item = item->next;
            }
            return list_val;
        }
        
        case AST_DICT: {
            Value dict_val;
            dict_val.type = VALUE_DICT;
            dict_val.as.dict = malloc(sizeof(Dict));
            Dict *dict = dict_val.as.dict;
            dict->bucket_count = 16;
            dict->buckets = calloc(dict->bucket_count, sizeof(DictEntry*));
            dict->count = 0;
            
            ASTNode *pair = node->children;
            (void)pair;
            return dict_val;
        }
        
        case AST_INDEX: {
            Value collection = interpreter_eval(interpreter, node->left);
            Value index = interpreter_eval(interpreter, node->right);
            
            if (collection.type == VALUE_LIST && index.type == VALUE_NUMBER) {
                int idx = (int)index.as.number;
                List *list = collection.as.list;
                if (idx >= 0 && idx < (int)list->count) {
                    return list->items[idx];
                } else {
                    printf("Error: List index out of range: %d\n", idx);
                    interpreter->error_occurred = 1;
                }
            } else if (collection.type == VALUE_STRING && index.type == VALUE_NUMBER) {
                int idx = (int)index.as.number;
                char *str = collection.as.string;
                if (idx >= 0 && idx < (int)strlen(str)) {
                    char res[2];
                    res[0] = str[idx];
                    res[1] = '\0';
                    Value v; v.type = VALUE_STRING; v.as.string = strdup(res);
                    return v;
                }
            } else if (collection.type == VALUE_DICT && index.type == VALUE_STRING) {
                Dict *dict = collection.as.dict;
                char *key = index.as.string;
                unsigned long hash = 5381;
                int c;
                char *k = key;
                while ((c = *k++)) hash = ((hash << 5) + hash) + c;
                size_t bucket_idx = hash % dict->bucket_count;
                
                DictEntry *entry = dict->buckets[bucket_idx];
                while (entry) {
                    if (strcmp(entry->key, key) == 0) {
                        return *entry->value;
                    }
                    entry = entry->next;
                }
            }
            break;
        }
        
        case AST_MEMBER_ACCESS: {
            Value obj = interpreter_eval(interpreter, node->left);
            char *member_name = node->right->value;
            
            if (obj.type == VALUE_FILE) {
                BuiltinFunc func = NULL;
                if (strcmp(member_name, "write") == 0) func = (BuiltinFunc)stdlib_file_write;
                else if (strcmp(member_name, "read") == 0) func = (BuiltinFunc)stdlib_file_read;
                else if (strcmp(member_name, "close") == 0) func = (BuiltinFunc)stdlib_file_close;
                
                if (func) {
                    Value args[16];
                    args[0] = obj;
                    int argc = 1;
                    
                    if (node->children || (node->value && strcmp(node->value, "call") == 0)) {
                        ASTNode *arg_node = node->children;
                        while (arg_node && argc < 16) {
                            args[argc++] = interpreter_eval(interpreter, arg_node);
                            arg_node = arg_node->next;
                        }
                    }
                    
                    return func(args, argc);
                }
            }

            if (obj.type == VALUE_OBJECT) {
                Dict *dict = obj.as.dict;
                unsigned long hash = 5381;
                int c;
                char *k = member_name;
                while ((c = *k++)) hash = ((hash << 5) + hash) + c;
                size_t bucket_idx = hash % dict->bucket_count;
                
                DictEntry *entry = dict->buckets[bucket_idx];
                while (entry) {
                    if (strcmp(entry->key, member_name) == 0) {
                        Value val = *entry->value;
                        
                        if (node->children || (node->value && strcmp(node->value, "call") == 0)) { 
                             if (val.type == VALUE_FUNCTION) {
                                ASTNode *func_node = val.as.function;
                                
                                if (!func_node) {
                                    BuiltinFunc builtin = get_builtin(member_name);
                                    if (builtin) {
                                        Value args[16];
                                        args[0] = obj;
                                        int argc = 1;
                                        
                                        ASTNode *arg_node = node->children;
                                        while (arg_node && argc < 16) {
                                            args[argc++] = interpreter_eval(interpreter, arg_node);
                                            arg_node = arg_node->next;
                                        }
                                        return builtin(args, argc);
                                    }
                                    return (Value){VALUE_NULL, {0}};
                                }

                                Value args[16];
                                int argc = 0;
                                ASTNode *arg_node = node->children;
                                while (arg_node && argc < 16) {
                                    args[argc++] = interpreter_eval(interpreter, arg_node);
                                    arg_node = arg_node->next;
                                }
                                
                                interpreter_push_scope(interpreter);
                                
                                ASTNode *param = func_node->left;
                                int arg_idx = 0;
                                while (param && arg_idx < argc) {
                                    if (param->value) {
                                        interpreter_set_variable(interpreter, param->value, args[arg_idx]);
                                    }
                                    param = param->next;
                                    arg_idx++;
                                }
                                
                                ASTNode *body = func_node->children;
                                Value res = {VALUE_NULL, {0}};
                                
                                if (body && (body->type == AST_BLOCK || body->type == AST_INNER_BLOCK)) {
                                    ASTNode *stmt = body->children;
                                    while (stmt) {
                                        res = interpreter_eval(interpreter, stmt);
                                        Value ret_check = interpreter_get_variable(interpreter, "__return__");
                                        if (ret_check.type == VALUE_NUMBER && ret_check.as.number == 1) {
                                            res = interpreter_get_variable(interpreter, "__return_value__");
                                            interpreter_set_variable(interpreter, "__return__", (Value){VALUE_NULL, {0}});
                                            interpreter_pop_scope(interpreter);
                                            return res;
                                        }
                                        if (stmt->type == AST_BREAK || stmt->type == AST_CONTINUE) break;
                                        stmt = stmt->next;
                                    }
                                } else if (body) {
                                    res = interpreter_eval(interpreter, body);
                                    Value ret_check = interpreter_get_variable(interpreter, "__return__");
                                    if (ret_check.type == VALUE_NUMBER && ret_check.as.number == 1) {
                                        res = interpreter_get_variable(interpreter, "__return_value__");
                                        interpreter_set_variable(interpreter, "__return__", (Value){VALUE_NULL, {0}});
                                    }
                                }
                                
                                interpreter_pop_scope(interpreter);
                                return res;
                             }
                        }
                        
                        return val;
                    }
                    entry = entry->next;
                }
            }
            break;
        }
        
        case AST_FUNCTION_DEF: {
            Value val;
            val.type = VALUE_FUNCTION;
            val.as.function = node;
            interpreter_set_variable(interpreter, node->value, val);
            break;
        }
        
        case AST_CLASS_DEF: {
            Value val;
            val.type = VALUE_CLASS;
            val.as.class_def = node;
            interpreter_set_variable(interpreter, node->value, val);
            break;
        }
        
        case AST_FUNCTION_CALL:
            return interpreter_call_function(interpreter, node);
            
        case AST_MAIN_CALL:
            return interpreter_call_function(interpreter, node);
            
        case AST_HTTP_REQUEST: {
            if (!node->left || !node->right) {
                Value error_val;
                error_val.type = VALUE_STRING;
                error_val.as.string = strdup("HTTP Error: Missing method or URL");
                return error_val;
            }
            
            Value method_val = interpreter_eval(interpreter, node->left);
            Value url_val = interpreter_eval(interpreter, node->right);
            
            if (method_val.type != VALUE_STRING || url_val.type != VALUE_STRING) {
                Value error_val;
                error_val.type = VALUE_STRING;
                error_val.as.string = strdup("HTTP Error: Method and URL must be strings");
                if (method_val.type == VALUE_STRING) free(method_val.as.string);
                if (url_val.type == VALUE_STRING) free(url_val.as.string);
                return error_val;
            }
            
            char *method = method_val.as.string;
            char *url = url_val.as.string;
            char *data = NULL;
            const char **headers = NULL;
            int header_count = 0;
            
            if (node->children) {
                Value data_val = interpreter_eval(interpreter, node->children);
                if (data_val.type == VALUE_STRING) {
                    data = data_val.as.string;
                    
                    if (strcmp(method, "POST") == 0 || strcmp(method, "PUT") == 0) {
                        headers = malloc(sizeof(char*) * 1);
                        headers[0] = "Content-Type: application/json";
                        header_count = 1;
                    }
                } else {
                    free(data_val.as.string);
                }
            }
            
            char *response = http_request(method, url, data, headers, header_count);
            
            if (!response) {
                Value error_val;
                error_val.type = VALUE_STRING;
                error_val.as.string = strdup("HTTP Error: http_request returned NULL");
                free(method_val.as.string);
                free(url_val.as.string);
                if (data) free(data);
                if (headers) free(headers);
                return error_val;
            }
            
            Value result;
            result.type = VALUE_STRING;
            result.as.string = response;
            
            free(method_val.as.string);
            free(url_val.as.string);
            if (data) free(data);
            if (headers) free(headers);
            
            return result;
        }

        case AST_START: {
             break;
        }
        
        case AST_IMPORT: {
            if (node->value) {
                ASTNode *module_ast = module_load(node->value);
                if (module_ast) {
                    module_execute(interpreter, module_ast);
                }
            }
            break;
        }
            
        default:
            break;
    }
    
    Value null_value = {VALUE_NULL, {0}};
    return null_value;
}

Value interpreter_call_function(Interpreter *interpreter, ASTNode *node) {
    char *func_name = strdup("main");
    
    if (node->type == AST_FUNCTION_CALL && node->value) {
        if (strlen(node->value) > 2 && node->value[0] == '>' && node->value[strlen(node->value)-1] == '<') {
            size_t len = strlen(node->value) - 2;
            free(func_name);
            func_name = malloc(len + 1);
            memcpy(func_name, node->value + 1, len);
            func_name[len] = '\0';
        } else {
            free(func_name);
            func_name = strdup(node->value);
        }
    } else {
    }
    
    BuiltinFunc builtin = get_builtin(func_name);
    if (builtin) {
        Value args[16];
        int argc = 0;
        ASTNode *arg_node = node->left;
        while (arg_node && argc < 16) {
            args[argc++] = interpreter_eval(interpreter, arg_node);
            arg_node = arg_node->next;
        }
        Value result = builtin(args, argc);
        free(func_name);
        return result;
    }
    
    Value func_value = interpreter_get_variable(interpreter, func_name);
    
    if (func_value.type == VALUE_FUNCTION && func_value.as.function) {
        ASTNode *func_node = func_value.as.function;
        
        Value args[16];
        int argc = 0;
        ASTNode *arg_node = node->left;
        while (arg_node && argc < 16) {
            args[argc++] = interpreter_eval(interpreter, arg_node);
            arg_node = arg_node->next;
        }
        
        if (func_node->children) {
            interpreter_push_scope(interpreter);
            
            ASTNode *param = func_node->left;
            int arg_idx = 0;
            while (param && arg_idx < argc) {
                if (param->value) {
                    interpreter_set_variable(interpreter, param->value, args[arg_idx]);
                }
                param = param->next;
                arg_idx++;
            }
            
            ASTNode *body = func_node->children;
            
            if (body->type == AST_BLOCK || body->type == AST_INNER_BLOCK) {
                ASTNode *stmt = body->children;
                Value result = {VALUE_NULL, {0}};
                while (stmt) {
                    result = interpreter_eval(interpreter, stmt);
                    Value ret_check = interpreter_get_variable(interpreter, "__return__");
                    if (ret_check.type == VALUE_NUMBER && ret_check.as.number == 1) {
                        result = interpreter_get_variable(interpreter, "__return_value__");
                        interpreter_pop_scope(interpreter);
                        interpreter_set_variable(interpreter, "__return__", ret_check);
                        interpreter_set_variable(interpreter, "__return_value__", result);
                        free(func_name);
                        fflush(stdout);
                        return result;
                    }
                    if (stmt->type == AST_BREAK || stmt->type == AST_CONTINUE) break;
                    stmt = stmt->next;
                }
                interpreter_pop_scope(interpreter);
                free(func_name);
                fflush(stdout);
                return result;
            } else {
                Value result = interpreter_eval(interpreter, body);
                Value ret_check = interpreter_get_variable(interpreter, "__return__");
                if (ret_check.type == VALUE_NUMBER && ret_check.as.number == 1) {
                    result = interpreter_get_variable(interpreter, "__return_value__");
                    interpreter_pop_scope(interpreter);
                    interpreter_set_variable(interpreter, "__return__", ret_check);
                    interpreter_set_variable(interpreter, "__return_value__", result);
                    free(func_name);
                    fflush(stdout);
                    return result;
                }
                interpreter_pop_scope(interpreter);
                free(func_name);
                fflush(stdout);
                return result;
            }
        } else {
            if (node->type == AST_MAIN_CALL) {
            }
        }
    } else {
        if (node->type == AST_MAIN_CALL) {
            printf("Error: main function not found\n"); 
        } else {
            printf("Warning: function '%s' not found\n", func_name);
        }
        fflush(stdout);
    }
    
    free(func_name);
    Value null_value = {VALUE_NULL, {0}};
    return null_value;
}
