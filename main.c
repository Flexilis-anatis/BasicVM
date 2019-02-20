#include "comp.h"
#include "runner.h"
#include "dis.h"
#include <stdio.h>
#include <readline/readline.h>

int main(void) {
    char *expr = readline("EXPR> ");
    Chunk chunk = compile(expr);
    emit_byte(&chunk, OP_RETURN);

    dis(&chunk);

    Scope *scope = init_scope(&chunk);
    VM *vm = init_vm(scope);

    run_vm(vm);
    
    free_vm(vm);
    free(expr);

    return 0;
}

