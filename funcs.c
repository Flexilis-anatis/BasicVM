#include "funcs.h"
#include "value.h"
#include "token.h"
#include "func.h"
#include "closure.h"
#include "runner.h"
#include <math.h>
#include <assert.h>
#include <string.h>

#ifdef DEBUG_RUNNER
#include "dis.h"
#endif

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
    return scope->chunk->consts->data[index];
}

void pop_back(Scope *scope) {
    free_value(pop(scope));
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
#define LOG_FN(name,op) OPER_FN(name, p = bool_value(x->d op y.d).p)
LOG_FN(lt, <)

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
    unsigned long index = extract_number(&scope->ip);
    scope->ip += scope->chunk->consts->jumps[index];
}

void op_cond_jmp(Scope *scope) {
    Value cond = pop(scope);
    unsigned long index = extract_number(&scope->ip);
    if (!value_true(cond))
        scope->ip += scope->chunk->consts->jumps[index];
    free_value(cond);
}

static void store(Scope *scope, Lex ident, Value value) {
    size_t size = ident.end-ident.start;
    ht_insert(scope->local_vars, ident.start, size, value);
}

static Lex get_ident(Scope *scope) {
    size_t ident_key = extract_number(&scope->ip);
    Lex ident = scope->chunk->consts->idents[ident_key];
    return ident;
}

void op_store(Scope *scope) {
    Lex ident = get_ident(scope);

    Value val = pop(scope);
    store(scope, ident, val);
    vector_push_back(scope->stack, copy_val(val));
}

void op_const_store(Scope *scope) {
    Lex ident = get_ident(scope);

    size_t value_key = extract_number(&scope->ip);
    store(scope, ident, scope->chunk->consts->data[value_key]);
    vector_push_back(scope->stack, copy_val(scope->chunk->consts->data[value_key]));
}

void op_load(Scope *scope) {
    Lex ident = get_ident(scope);
    size_t size = ident.end-ident.start;

    Scope *cur_scope = scope;
    Value *result = NULL;
    while (result == NULL && cur_scope != NULL) {
        result = ht_get(cur_scope->local_vars, ident.start, size);
        cur_scope = (Scope *)cur_scope->parent;
    }

    if (result == NULL)
        RUNTIME_ERROR("Variable not found", -8);
    else
        push_val(scope, copy_val(*result));
}

Scope *make_scope(size_t call_arity, Func *func, Scope *scope) {
    Scope *new_scope = init_scope(&func->chunk);
    new_scope->parent = scope;

    assert(call_arity == func_arity(func));
    while (call_arity--) {
        Lex ident = func->params[call_arity];
        store(new_scope, ident, pop(scope));
    }
    
    return new_scope;
}

void func_call(Scope *scope, Value func_val, size_t call_arity) {
    Func *func = VAL_AS(func_val, Func *);
    
    Scope *fscope = make_scope(call_arity, func, scope);

    InterpStat result = run_scope(fscope);
    vector_pop_back(scope->stack);
    #ifdef DEBUG_RUNNER
    print_stack(scope);
    #endif
    if (result == INTERP_END || vector_size(fscope->stack) == 0) {
        push_val(scope, nil_val());
    } else {
        push_val(scope, copy_val(last_val(fscope)));
    }

    free_scope(fscope);
    vector_free(func->params);
    free(func);
}

void closure_call(Scope *scope, Value close_val, size_t call_arity) {
    Closure *closure = VAL_AS(close_val, Closure *);

    Scope *fscope = malloc(sizeof(Scope));
    fscope->chunk = &closure->func->chunk;
    fscope->ip = fscope->chunk->code;
    fscope->local_vars = closure->ht;
    fscope->parent = scope;
    fscope->stack = NULL;

    assert(call_arity == func_arity(closure->func));
    while (call_arity--) {
        Lex ident = closure->func->params[call_arity];
        store(fscope, ident, pop(scope));
    }

    InterpStat result = run_scope(fscope);
    vector_pop_back(scope->stack);
    #ifdef DEBUG_RUNNER
    print_stack(scope);
    #endif
    if (result == INTERP_END || vector_size(fscope->stack) == 0) {
        push_val(scope, nil_val());
    } else {
        push_val(scope, copy_val(last_val(fscope)));
    }

    free_scope(fscope);
    vector_free(closure->func->params);
    free(closure->func);
    free(closure);
}

void op_call(Scope *scope) {
    size_t call_arity = extract_number(&scope->ip);
    Value val = scope->stack[vector_size(scope->stack)-call_arity-1];
    switch (GET_TYPE(val)) {
    case TYPE_FUNC:
        func_call(scope, val, call_arity);
        break;
    case TYPE_CLOSURE:
        closure_call(scope, val, call_arity);
        break;
    }
}

void op_bind(Scope *scope) {
    size_t index = extract_number(&scope->ip);
    Value val = scope->chunk->consts->data[index];
    Closure *close = VAL_AS(val, Closure *);
    hash_table tmp = ht_copy(scope->local_vars, copy_val);
    close->ht = malloc(sizeof(hash_table));
    memcpy(close->ht, &tmp, sizeof(hash_table));
    push_val(scope, copy_val(val));
}