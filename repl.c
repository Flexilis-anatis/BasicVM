#include "repl.h"
#include "vm.h"
#include "comp.h"
#include <readline/readline.h>
#include <string.h>

// I suck at string programming, so I'm just reallocating everything

char *add_newline(char *str) {
    size_t len = strlen(str);
    char *new = calloc(len+1,1);
    ++str;
    strcpy(new, str);
    free(str-1);
    new[len-1] = '\n';
    return new;
}

char *add_line(char *lhs, char *rhs) {
    size_t len1 = strlen(lhs), len2 = strlen(rhs);
    char *new = calloc(len1+len2+2, 1);
    strcpy(new, lhs);
    strcat(new, rhs);
    free(lhs);
    free(rhs);
    new[len1+len2] = '\n';
    return new;
}

char *get(void) {
    char *inp = readline("BVM> ");
    if (*inp == '\0') {
        free(inp);
        return get();
    }

    if (*inp == '$') {
        inp = add_newline(inp);
        size_t size = strlen(inp);
        while (size < 2 || inp[size-2] != '$') {
            char *new = readline("      ");
            inp = add_line(inp, new);
            size = strlen(inp);
        }
        inp[size-2] = '\0';
    }
    add_history(inp);
    return inp;
}

void run_repl(void) {
    while(1) {
        char *code = get();
        printf("%s", code);
        free(code);
    }
}