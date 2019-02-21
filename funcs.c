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

void pop_back(Scope *scope) {
    vector_pop_back(scope->stack);
    free_value(*vector_end(scope->stack));
}

Value pop(Scope *scope) {
    vector_pop_back(scope->stack);
    return *vector_end(scope->stack);
}

#define OPER_FN(name, op) \
    void op_##name(Scope *scope) {            \
        Value y = last_val(scope),            \
             *x = vector_end(scope->stack)-2; \
        x-> op;                                \
        vector_pop_back(scope->stack);        \
    }

OPER_FN(add, d += y.d)
OPER_FN(sub, d -= y.d)
OPER_FN(mul, d *= y.d)
OPER_FN(div, d /= y.d)
OPER_FN(mod, d = fmod(x->d, y.d))

void op_print(Scope *scope) {
    Value val = pop(scope);
    print_value(val);
    free_value(val);
}

void op_const_jmp(Scope *scope) {
    long number = (long)extract_number(&scope->ip);
    scope->ip += number > 0 ? number : -number;
}

void op_cond_jmp(Scope *scope) {
    Value cond = pop(scope);
    if (!value_true(cond))
        op_const_jmp(scope);
    free_value(cond);
}