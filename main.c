#include "comp.h"
#include "runner.h"
#include "dis.h"
#include "func.h"
#include "closure.h"
#include <stdio.h>
#include <string.h>

char *readfile(char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "File '%s' does not exist. Aborting.\n", filename);
        exit(500);
    }
    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buffer = malloc(length + 1);
    buffer[length] = '\0';
    fread(buffer, 1, length, f);
    fclose(f);
    return buffer;
}

int main(int argc, char **argv) {
    if (argc == 1) {
        puts("Usage: bvm [-d|--dis] file1 [file2 fileN]");
    } else {
        bool showdis = false;
        if (strcmp(argv[1], "-d") == 0 || strcmp(argv[1], "--dis") == 0)
            showdis = true;
        Chunk chunk = empty_chunk(empty_consts());
        Scope *scope = init_scope(&chunk);
        VM *vm = init_vm(scope);

        int arg = showdis;
        while (++arg < argc) {
            char *contents = readfile(argv[arg]);
            free_vm(vm);
            chunk = compile(contents);
            if (showdis) {
                dis(&chunk);
                puts("-------------------");
                for (size_t i = 0; i < vector_size(chunk.consts->data); ++i) {
                    Func *f;
                    Value v = chunk.consts->data[i];
                    if (!IS_POINTER(v))
                        continue;
                    switch(GET_TYPE(v)) {
                    case TYPE_CLOSURE:
                        printf("Closure");
                        f = VAL_AS(v, Closure *)->func;
                        break;
                    case TYPE_FUNC:
                        printf("Function");
                        f = VAL_AS(v, Func *);
                        break;
                    default:
                        continue;
                    }
                    putchar('(');
                    if (vector_size(f->params)) {
                        for (size_t p = 0; p < vector_size(f->params)-1; ++p) {
                            Lex ident = f->params[p];
                            for (const char *c = ident.start; c != ident.end; ++c)
                                putchar(*c);
                            printf(", ");
                        }
                        Lex ident = f->params[vector_size(f->params)-1];
                        for (const char *c = ident.start; c != ident.end; ++c)
                            putchar(*c);
                    }
                    puts(")");
                    dis(&f->chunk);
                    puts("-------------------");
                }
            }
            scope = init_scope(&chunk);
            vm = init_vm(scope);
            run_vm(vm);
            free(contents);
        }
        free_vm(vm);
    }

    return 0;
}

