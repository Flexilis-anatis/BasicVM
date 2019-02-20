#include "funcs.h"
#include <math.h>

void push_val(Scope *scope, Value value) {
    vector_push_back(scope->stack, value);
}

Value last_val(Scope *scope) {
    return *(vector_end(scope->stack)-1);
}

Value get_val(Scope *scope) {
    unsigned long index = extract_number(&scope->ip);
    return scope->chunk->data[index];
}

#define OPER_FN(name, op) \
    void op_##name(Scope *scope) {            \
        Value y = last_val(scope),            \
             *x = vector_end(scope->stack)-2; \
        *x op;                                \
        vector_pop_back(scope->stack);        \
    }

OPER_FN(add, += y)
OPER_FN(sub, -= y)
OPER_FN(mul, *= y)
OPER_FN(div, /= y)
OPER_FN(mod, = fmod(*x, y))
