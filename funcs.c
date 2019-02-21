#include "funcs.h"
#include "value.h"
#include "token.h"
#include <math.h>

#define RUNTIME_ERROR(m,c) \
    do {                                   \
        fprintf(stderr,                    \
              "Runtime error: %s\n", (m)); \
        exit(c);                           \
    } while (0)

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
    putchar(' ');
}

void op_puts(Scope *scope) {
    op_print(scope);
    putchar('\n');
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

static void store(Scope *scope, const char *key, size_t key_size, Value value) {
    ht_insert(scope->local_vars, key, key_size, value);
    vector_push_back(scope->stack, value);
}

static Lex get_ident(Scope *scope) {
    size_t ident_key = extract_number(&scope->ip);
    Lex ident = scope->chunk->idents[ident_key];
    return ident;
}

void op_store(Scope *scope) {
    Lex ident = get_ident(scope);
    size_t size = ident.end-ident.start;

    store(scope, ident.start, size, last_val(scope));
}

void op_const_store(Scope *scope) {
    Lex ident = get_ident(scope);
    size_t size = ident.end-ident.start;

    size_t value_key = extract_number(&scope->ip);
    store(scope, ident.start, size, scope->chunk->data[value_key]);
}

void op_load(Scope *scope) {
    Lex ident = get_ident(scope);
    size_t size = ident.end-ident.start;

    Scope *cur_scope = scope;
    Value *result;
    while (result == NULL && cur_scope != NULL) {
        result = ht_get(cur_scope->local_vars, ident.start, size);
        cur_scope = cur_scope->parent;
    }
    if (result == NULL)
        RUNTIME_ERROR("Variable not found", -8);
    else
        push_val(scope, *result);
}