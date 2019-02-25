#pragma once

#include "func.h"

typedef struct {
    Func *func;
    hash_table *ht;
} Closure;

void free_closure(Closure *closure);
Closure *copy_closure(Closure *closure);