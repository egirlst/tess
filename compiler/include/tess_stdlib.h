#ifndef TESS_STDLIB_H
#define TESS_STDLIB_H

#include "interpreter.h"

Value stdlib_print(Value *args, int argc);
Value stdlib_sqrt(Value *args, int argc);
Value stdlib_len(Value *args, int argc);
Value stdlib_file_open(Value *args, int argc);
Value stdlib_file_write(Value *args, int argc);
Value stdlib_file_read(Value *args, int argc);
Value stdlib_file_close(Value *args, int argc);
Value stdlib_read_file(Value *args, int argc);
Value stdlib_write_file(Value *args, int argc);
Value stdlib_mem_alloc(Value *args, int argc);
Value stdlib_mem_free(Value *args, int argc);
Value stdlib_mem_write(Value *args, int argc);
Value stdlib_mem_read(Value *args, int argc);
Value stdlib_sys_sleep(Value *args, int argc);
Value stdlib_sys_exit(Value *args, int argc);
Value stdlib_asm_alloc_exec(Value *args, int argc);
Value stdlib_asm_exec(Value *args, int argc);
Value stdlib_json_format(Value *args, int argc);
Value stdlib_clock(Value *args, int argc);

#endif
