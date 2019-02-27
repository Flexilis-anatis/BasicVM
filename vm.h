#pragma once
#include "chunk.h"
#include "token.h"
#include "ht/hashtable.h"

typedef struct _sScope *ScopePtr;
typedef struct _sScope {
    vector(Value) stack;
    hash_table *local_vars;

    ScopePtr parent;

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