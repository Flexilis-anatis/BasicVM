#include "runner.h"
#include "funcs.h"

// The VM object just exists for slightly more extendability
void run_vm(VM *vm) {
    run_scope(vm->scope);
}

void run_scope(Scope *scope) {
    while (run_next(scope));
}

// Run the next instruction
bool run_next(Scope *scope) {
    if (scope->ip >= vector_end(scope->chunk->code))
        return false;
    uint8_t instr = *(scope->ip++);
    switch (instr) {
        case OP_PUSH:
            push_val(scope, copy_val(get_val(scope)));
            break;

        case OP_NEG:
            scope->stack[vector_size(scope->stack)-1].d *= -1;
            break;

        case OP_LOAD:
            op_load(scope);
            break;

        case OP_STORE:
            op_store(scope);
            break;

        case OP_CONST_STORE:
            op_const_store(scope);
            break;

        case OP_PRINT:
            op_print(scope);
            break;

        case OP_PUTS:
            op_puts(scope);
            break;

        case OP_ADD:
            op_add(scope);
            break;

        case OP_SUB:
            op_sub(scope);
            break;

        case OP_DIV:
            op_div(scope);
            break;

        case OP_MOD:
            op_mod(scope);
            break;

        case OP_MUL:
            op_mul(scope);
            break;

        case OP_COND_JMP:
            op_cond_jmp(scope);
            break;

        case OP_CONST_JMP:
            op_const_jmp(scope);
            break;

        case OP_POP_TOP:
            pop_back(scope);
            break;

        case OP_RETURN:
            print_value(last_val(scope));
            putchar('\n');
            return false;
    }
    return true;
}