#include "comp.h"
#include <stdio.h>
#define EXPR(prec) expression(chunk, source, prec)
#define ERROR(msg, code) \
    do {                      \
        fprintf(stderr, msg); \
        exit(code);           \
    } while (0)

// Similar to a Pratt-parser, but using a switch-case statement
void expression(Chunk *chunk, Source *source, Prec prec);

// Emit a constant. Makes sure it's not a repeat
void emit_constant(Chunk *chunk, Value number) {
    // Check to see if it's a repeat constant
    for (size_t i = 0; i < vector_size(chunk->data); ++i) {
        if (chunk->data[i] == number) {
            emit_number(chunk, i);
            return;
        }
    }
    // If it's not a dup, add it to the data section AND emit it
    emit_value(chunk, number);
}

void parse_atom(Chunk *chunk, Source *source) {
    if (peek_token(source).id == TOK_NUMBER) {
        Token numtok = next_token(source);
        double number = strtod(numtok.lex.start, NULL);

        emit_byte(chunk, OP_PUSH);
        emit_constant(chunk, number);
    }
}

void parse_paren(Chunk *chunk, Source *source) {
    if (match_token(source, TOK_LPAREN)) {
        EXPR(PREC_NONE);
        if (!match_token(source, TOK_RPAREN))
            ERROR("No closing parenthesis", -1);
    } else {
        EXPR(PREC_PAREN+1);
    }
}

void parse_negate(Chunk *chunk, Source *source) {
    if (match_token(source, TOK_SUB)) {
        EXPR(PREC_NEGATE+1);
        emit_byte(chunk, OP_NEG);
    } else {
        EXPR(PREC_NEGATE+1);
    }
}

void parse_multiplication(Chunk *chunk, Source *source) {
    EXPR(PREC_MUL+1);
    uint8_t op;
    switch (peek_token(source).id) {
        case TOK_MOD:
            op = OP_MOD;
            break;
        case TOK_DIV:
            op = OP_DIV;
            break;
        case TOK_MUL:
            op = OP_MUL;
            break;
        default:
            return;
    }
    consume(source);

    EXPR(PREC_MUL+1);
    emit_byte(chunk, op);
}

void parse_addition(Chunk *chunk, Source *source) {
    EXPR(PREC_ADD+1);
    uint8_t op;
    switch (peek_token(source).id) {
        case TOK_ADD:
            op = OP_ADD;
            break;
        case TOK_SUB:
            op = OP_SUB;
            break;
        default:
            return;
    }
    consume(source);

    EXPR(PREC_ADD+1);
    emit_byte(chunk, op);
}

void expression(Chunk *chunk, Source *source, Prec prec) {
    switch (prec) {
        case PREC_NONE:
        case PREC_ADD:
            parse_addition(chunk, source);
            break;
        case PREC_MUL:
            parse_multiplication(chunk, source);
            break;
        case PREC_NEGATE:
            parse_negate(chunk, source);
            break;
        case PREC_PAREN:
            parse_paren(chunk, source);
            break;
        case PREC_ATOM:
            parse_atom(chunk, source);
            break;
    }
}

// Probably should make it take a Source object, but hey
Chunk compile(const char *code) {
    Source source = init_source(code);

    Chunk chunk = empty_chunk();
    expression(&chunk, &source, PREC_NONE);
    return chunk;
}
