#pragma once
#include "chunk.h"
#include "ht/hashtable.h"

typedef struct scope_struct_name {
    vector(Value) stack;
    hash_table *local_vars;
    struct scope_struct_name *parent_scope;
    Chunk *chunk;
    uint8_t *ip;
} Scope;

typedef struct {
    Scope *scope; // only really need one for an expression parser
} VM;

VM *init_vm(Scope *main_scope);
void free_vm(VM *vm);
Scope *init_scope(Chunk *chunk);
void free_scope(Scope *scope);