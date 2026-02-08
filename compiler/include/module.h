#ifndef MODULE_H
#define MODULE_H

#include "interpreter.h"

ASTNode* module_load(const char *module_name);
void module_execute(Interpreter *interpreter, ASTNode *module_ast);

#endif
