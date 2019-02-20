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
    uint8_t instr = *(scope->ip++);
    switch (instr) {
        case OP_PUSH:
            push_val(scope, get_val(scope));
            break;

        case OP_NEG:
            // One liners are fine
            scope->stack[vector_size(scope->stack)-1] *= -1;
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

        case OP_RETURN: // again, just extendability
            print_value(last_val(scope));
            putchar('\n');
            return false;
    }
    return true;
}