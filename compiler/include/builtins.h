#ifndef BUILTINS_H
#define BUILTINS_H

#include "interpreter.h"

void register_builtins(Interpreter *interpreter);
BuiltinFunc get_builtin(const char *name);

#endif
