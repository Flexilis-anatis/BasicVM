#include "class.h"
#include <string.h>

Instance *instantiate(Class *cls) {
    Instance *inst = malloc(sizeof(Instance));
    inst->cls = cls;
    inst->ht = malloc(sizeof(hash_table));
    ht_init(inst->ht, 0.03);
    return inst;
}

void free_instance(Instance *inst) {
    ht_free(inst->ht);
    free(inst);
}

void free_class(Class *cls) {
    free_value(cls->new_func);
    ht_free(cls->ht);
    free(cls);
}

Class *copy_class(Class *cls) {
    Class *new = malloc(sizeof(Class));
    
    new->ht = malloc(sizeof(hash_table));
    hash_table tmp = ht_copy(new->ht, copy_val);
    memcpy(new->ht, &tmp, sizeof(hash_table));

    new->name = cls->name;
    new->new_func = copy_val(cls->new_func);
    new->super = cls->super;

    return new;
}

Instance *copy_instance(Instance *inst) {
    Instance *new = malloc(sizeof(Instance));

    new->ht = malloc(sizeof(hash_table));
    hash_table tmp = ht_copy(new->ht, copy_val);
    memcpy(new->ht, &tmp, sizeof(hash_table));

    new->cls = inst->cls;

    return new;
}