#include "vm.h"

Scope *init_scope(Chunk *chunk) {
    Scope *scope = malloc(sizeof(Scope));
    scope->chunk = chunk;
    scope->stack = NULL;
    scope->ip = chunk->code;
    return scope;
}

void free_scope(Scope *scope) {
    free_chunk(scope->chunk);
    vector_free(scope->stack);
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