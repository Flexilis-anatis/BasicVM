#include "vm.h"

void free_value_voidp(void *p) {
    free_value(*(Value *)p);
}

Scope *init_scope(Chunk *chunk) {
    Scope *scope = malloc(sizeof(Scope));

    scope->chunk = chunk;
    scope->stack = NULL;
    scope->parent_scope = NULL;
    scope->ip = chunk->code;

    scope->local_vars = malloc(sizeof(hash_table));
    ht_init(scope->local_vars, 0, 0.03, free_value_voidp);

    return scope;
}

void free_scope(Scope *scope) {
    free_chunk(scope->chunk);
    vector_free(scope->stack);
    ht_free(scope->local_vars);
    free(scope);
}

VM *init_vm(Scope *scope) {
    VM *vm = malloc(sizeof(VM));
    vm->scope = scope;
    return vm;
}

void free_vm(VM *vm) {
    free_scope(vm->scope);
    free(vm);
}