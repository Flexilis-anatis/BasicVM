#include "comp.h"
#include "runner.h"
#include "dis.h"
#include <stdio.h>
#include <readline/readline.h>

/*Chunk chunk = compile("(-200*3+50)/2");
    emit_byte(&chunk, OP_RETURN);

    dis(&chunk);

    Scope *scope = init_scope(&chunk);
    VM *vm = init_vm(scope);

    run_vm(vm);
    free_vm(vm);*/

int main(void) {
    Chunk chunk = empty_chunk();

    emit_byte(&chunk, OP_PUSH);
    emit_value(&chunk, 25.3);
    emit_byte(&chunk, OP_RETURN);

    dis(&chunk);

    Scope *scope = init_scope(&chunk);
    VM *vm = init_vm(scope);

    run_vm(vm);
    free_vm(vm);

    return 0;
}

