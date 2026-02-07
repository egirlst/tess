#ifndef BUILTINS_H
#define BUILTINS_H

#include "interpreter.h"

typedef Value (*BuiltinFunc)(Value *args, int argc);

BuiltinFunc get_builtin(const char *name);
void register_builtins(Interpreter *interpreter);

#endif /* BUILTINS_H */

