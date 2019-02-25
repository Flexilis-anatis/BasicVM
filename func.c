#include "func.h"

void free_func(Func *func) {
    free_chunk(&func->chunk);
    vector_free(func->params);
    free(func);
}

Func *copy_func(Func *func) {
    Func *new = malloc(sizeof(Func));
    if (new == NULL) {
        fprintf(stderr, "ERROR: MEMORY FULL\n");
        exit(200);
    }
    
    new->params = NULL;
    vector_grow(new->params, vector_capacity(func->params));
    for (size_t i = 0; i < vector_size(func->params); ++i) {
        Lex item = func->params[i];
        vector_push_back(new->params, item);
    }

    new->chunk = empty_chunk(func->chunk.consts);
    vector_grow(new->chunk.code, vector_capacity(func->chunk.code));
    for (size_t i = 0; i < vector_size(func->chunk.code); ++i)
        vector_push_back(new->chunk.code, func->chunk.code[i]);
    return new;
}

size_t func_arity(Func *func) {
    return vector_size(func->params);
}