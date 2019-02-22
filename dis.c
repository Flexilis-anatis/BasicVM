#include "dis.h"

#define OP(op) \
    case OP_##op: {  \
        ip++;        \
        printf(#op); \
        break;       \
    }

void dis(Chunk *chunk) {
    uint8_t *ip = chunk->code;

    while (ip < vector_end(chunk->code)) {
        printf("@%lu: ", ip-chunk->code);
        switch (*ip) {
            case OP_CONST_JMP: {
                ip++;
                size_t number = extract_number(&ip);
                printf("CONSTJMP %lu\n", chunk->jumps[number]);
                break;
            }
            case OP_COND_JMP: {
                ip++;
                size_t number = extract_number(&ip);
                printf("JMP %lu\n", chunk->jumps[number]);
                break;
            }
            case OP_STORE: {
                ip++;
                printf("STORE ");
                size_t var = extract_number(&ip);
                Lex ident = chunk->idents[var];
                for (const char *i = ident.start; i < ident.end; ++i)
                    putchar(*i);
                putchar('\n');
                break;
            }
            case OP_PUTS: {
                printf("PUTS\n");
                ip++;
                break;
            }
            case OP_POP_TOP: {
                printf("POP\n");
                ip++;
                break;
            }
            OP(ADD)
            OP(MUL)
            OP(MOD)
            OP(DIV)
            OP(SUB)
            case OP_PUSH: {
                ip++;
                printf("PUSH ");
                size_t val = extract_number(&ip);
                print_value(chunk->data[val]);
                putchar('\n');
                break;
            }
            case OP_LOAD: {
                ip++;
                printf("LOAD ");
                size_t var = extract_number(&ip);
                Lex ident = chunk->idents[var];
                for (const char *i = ident.start; i < ident.end; ++i)
                    putchar(*i);
                putchar('\n');
                break;
            }
            default: {
                printf("UNRECOGNIZED: %u\n", *ip++);
                break;
            }
        }
    }
}