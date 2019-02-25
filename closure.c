#include "closure.h"
#include <string.h> // memcpy

void free_closure(Closure *closure) {
    ht_free(closure->ht);
    free_func(closure->func);
    free(closure);
}

Closure *copy_closure(Closure *closure) {
    Closure *new = malloc(sizeof(Closure));
    if (new == NULL) {
        fprintf(stderr, "ERROR: MEMORY FULL\n");
        exit(200);
    }
    
    new->func = copy_func(closure->func);
    hash_table tmp = ht_copy(closure->ht, copy_val);
    new->ht = malloc(sizeof(hash_table));
    memcpy(new->ht, &tmp, sizeof(hash_table));

    return new;
}
