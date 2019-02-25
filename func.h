#pragma once

#include "vm.h"

typedef struct {
    Chunk chunk;
    vector(Lex) params;
} Func;

void free_func(Func *func);
Func *copy_func(Func *func);
size_t func_arity(Func *func);