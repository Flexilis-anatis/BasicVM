#include "comp.h"
#include "runner.h"
#include "dis.h"
#include "func.h"
#include <stdio.h>
#include <readline/readline.h>
#include "asm.h"

#define STRING "x = 10; f = closure () {return x;}; x = 5; print f(), x;"
int main(void) {
    Chunk chunk = compile(STRING);
    dis(&chunk);
    printf("------------\n");
    //dis(&VAL_AS(chunk.consts->data[2], Func *)->chunk);
    printf("------------\n");
    Scope *scope = init_scope(&chunk);

    VM *vm = init_vm(scope);
    run_vm(vm);
    free_vm(vm);

    return 0;
}

