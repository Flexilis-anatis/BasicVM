#include "comp.h"
#include "runner.h"
#include "dis.h"
#include <stdio.h>
#include <readline/readline.h>

/**/

int main(void) {
    //char *expr = readline("EXPR> ");
    char *expr = "if (0) print \"here\"; else print \"to\"; ";
    Chunk chunk = compile(expr);
    
    dis(&chunk);

    Scope *scope = init_scope(&chunk);
    VM *vm = init_vm(scope);

    run_vm(vm);
    
    free_vm(vm);
    //free(expr);

    return 0;
}

