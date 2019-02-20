#include "comp.h"
#include "runner.h"
#include "dis.h"
#include <stdio.h>

int main(void) {
    Chunk chunk = compile("-(2 + 2) * 3");
    emit_byte(&chunk, OP_RETURN);

    dis(&chunk);

    Scope *scope = init_scope(&chunk);
    VM *vm = init_vm(scope);

    run_vm(vm);
    free_vm(vm);

    return 0;
}

