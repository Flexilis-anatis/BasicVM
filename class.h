#pragma once
#include "vm.h"
#include "func.h"
#include "value.h"

/*
PUSH 1
INSTANCE clas
*/

typedef struct _sClass {
    hash_table *ht;
    Value new_func;
    Lex name;
    struct _sClass *super;
} Class;

// Different instances all point to the same class, so they don't have to copy it's env
typedef struct {
    Class *cls;
    hash_table *ht;
} Instance;

Instance *instantiate(Class *cls);
void free_instance(Instance *inst);
void free_class(Class *cls);
Class *copy_class(Class *cls);
Instance *copy_instance(Instance *inst);