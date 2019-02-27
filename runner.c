#include "runner.h"
#include "funcs.h"

#ifdef DEBUG_RUNNER
#include "dis.h"
#endif

// The VM object just exists for slightly more extendability
void run_vm(VM *vm) {
    if (run_scope(vm->scope) == INTERP_RET) {
        fprintf(stderr, "Return statement outside of a function\n");
        exit(-19);
    }
}

InterpStat run_scope(Scope *scope) {
    InterpStat state;
    while ((state = run_next(scope)) == INTERP_OK);
    return state;
}

// Run the next instruction
InterpStat run_next(Scope *scope) {
    if (scope->ip >= vector_end(scope->chunk->code))
        return INTERP_END;

    #ifdef DEBUG_RUNNER
    uint8_t *fakeip = scope->ip;
    dis_instr(&fakeip, scope->chunk);
    getchar();
    #endif

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

        case OP_BIND:
            op_bind(scope);
            break;

        case OP_STORE:
            op_store(scope);
            break;

        case OP_CONST_STORE:
            op_const_store(scope);
            break;

        case OP_CALL:
            #ifdef DEBUG_RUNNER
            printf("Enter scope...\n");
            #endif
            op_call(scope);
            #ifdef DEBUG_RUNNER
            printf("Exit scope.\n");
            #endif
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

        case OP_AND:
            op_and(scope);
            break;

        case OP_OR:
            op_or(scope);
            break;

        case OP_NOT:
            op_not(scope);
            break;

        case OP_LT:
            op_lt(scope);
            break;

        case OP_LTE:
            op_lte(scope);
            break;
            
        case OP_GT:
            op_gt(scope);
            break;
            
        case OP_GTE:
            op_gte(scope);
            break;
        
        case OP_EQU:
            op_equ(scope);
            break;
            
        case OP_IS:
            op_is(scope);
            break;

        case OP_NEW:
            op_new(scope);
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
            return INTERP_RET;

        default:
            fprintf(stderr, "@%lu: UNRECOGNIZED OPCODE: %u\n", scope->ip-scope->chunk->code-1, instr);
            exit(-300);
    }
    #ifdef DEBUG_RUNNER
    print_stack(scope);
    #endif
    return INTERP_OK;
}