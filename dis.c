#include "dis.h"

#define OP(op) \
    case OP_##op: {    \
        (*ip)++;          \
        printf(#op);   \
        putchar('\n'); \
        break;         \
    }

void dis(Chunk *chunk) {
    uint8_t *ip = chunk->code;
    while (ip != vector_end(chunk->code))
        dis_instr(&ip, chunk);
}

void dis_instr(uint8_t **ip, Chunk *chunk) {
    printf("@%lu: ", *ip-chunk->code);
    switch (**ip) {
        case OP_CONST_JMP: {
            (*ip)++;
            size_t number = extract_number(ip);
            printf("CONSTJMP @%lu \n", (*ip-chunk->code)+chunk->consts->jumps[number]);
            break;
        }
        case OP_COND_JMP: {
            (*ip)++;
            size_t number = extract_number(ip);
            printf("JMP @%lu\n", (*ip-chunk->code)+chunk->consts->jumps[number]);
            break;
        }
        case OP_NEG: {
            (*ip)++;
            puts("NEG");
            break;
        }
        case OP_LT: {
            (*ip)++;
            puts("LT");
            break;
        }
        case OP_STORE: {
            (*ip)++;
            printf("STORE ");
            Lex ident = chunk->consts->idents[extract_number(ip)];
            for (const char *i = ident.start; i < ident.end; ++i)
                putchar(*i);
            putchar('\n');
            break;
        }
        case OP_PUTS: {
            printf("PUTS\n");
            (*ip)++;
            break;
        }
        case OP_PRINT: {
            printf("PRINT\n");
            (*ip)++;
            break;
        }
        case OP_POP_TOP: {
            printf("POP\n");
            (*ip)++;
            break;
        }
        case OP_CALL: {
            (*ip)++;
            printf("CALL %lu\n", extract_number(ip));
            break;
        }
        case OP_RETURN: {
            (*ip)++;
            puts("RETURN");
            break;
        }
        case OP_CONST_STORE: {
            (*ip)++;
            printf("CSTORE ");
            size_t var = extract_number(ip);
            Lex ident = chunk->consts->idents[var];
            for (const char *i = ident.start; i < ident.end; ++i)
                putchar(*i);
            size_t number = extract_number(ip);
            printf(" (");
            print_value(chunk->consts->data[number]);
            puts(")");
            break;
        }
        OP(ADD)
        OP(MUL)
        OP(MOD)
        OP(DIV)
        OP(SUB)
        case OP_PUSH: {
            (*ip)++;
            printf("PUSH ");
            size_t val = extract_number(ip);
            print_value(chunk->consts->data[val]);
            printf(" (%lu)\n", val);
            break;
        }
        case OP_LOAD: {
            (*ip)++;
            printf("LOAD ");
            Lex ident = chunk->consts->idents[extract_number(ip)];
            for (const char *i = ident.start; i < ident.end; ++i)
                putchar(*i);
            putchar('\n');
            break;
        }
        default: {
            printf("UNRECOGNIZED: %u\n", *(*ip++));
            break;
        }
    }
}

void print_stack(Scope *scope) {
    putchar('[');
    if (vector_size(scope->stack)) {
        for(size_t i = 0; i < vector_size(scope->stack)-1; ++i) {
            print_value(scope->stack[i]);
            printf(", ");
        }
        print_value(scope->stack[vector_size(scope->stack)-1]);
    }
    puts("]");
}