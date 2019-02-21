#include "comp.h"
#include <stdio.h>
#define EXPR(prec) expression(chunk, source, prec)
#define ERROR(msg, code) \
    do {                      \
        fprintf(stderr, msg); \
        exit(code);           \
    } while (0)

// Similar to a Pratt-parser, but using a switch-case statement
static void expression(Chunk *chunk, Source *source, Prec prec);

// Emit a constant. Makes sure it's not a repeat
static void emit_constant(Chunk *chunk, Value number) {
    // Check to see if it's a repeat constant
    for (size_t i = 0; i < vector_size(chunk->data); ++i) {
        if (same_value(chunk->data[i], number)) {
            emit_number(chunk, i);
            return;
        }
    }
    // If it's not a dup, add it to the data section AND emit it
    emit_value(chunk, number);
}

static void parse_atom(Chunk *chunk, Source *source) {
    if (peek_token(source).id == TOK_NUMBER) {
        Token numtok = next_token(source);
        double number = strtod(numtok.lex.start, NULL);

        emit_byte(chunk, OP_PUSH);
        emit_constant(chunk, double_val(number));
    }
}

static void parse_paren(Chunk *chunk, Source *source) {
    if (match_token(source, TOK_LPAREN)) {
        EXPR(PREC_NONE);
        if (!match_token(source, TOK_RPAREN))
            ERROR("No closing parenthesis", -1);
    } else {
        EXPR(PREC_PAREN+1);
    }
}

static void parse_negate(Chunk *chunk, Source *source) {
    if (match_token(source, TOK_SUB)) {
        EXPR(PREC_NEGATE);
        emit_byte(chunk, OP_NEG);
    } else {
        EXPR(PREC_NEGATE+1);
    }
}

static void parse_multiplication(Chunk *chunk, Source *source) {
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
    EXPR(PREC_NONE);
}

static void parse_addition(Chunk *chunk, Source *source) {
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
    EXPR(PREC_NONE);
}

static void parse_assignment(Chunk *chunk, Source *source) {
    // Is there an identifier?
    if (peek_token(source).id == TOK_IDENT) {
        // Save it
        Token var = next_token(source);
        // Is there an equals sign?
        if (match_token(source, TOK_ASSIGN)) {
            // Is the next token a number?
            if (peek_token(source).id == TOK_NUMBER) {
                Token num = next_token(source);
                size_t index = write_value(chunk, double_val(strtod(num.lex.start, NULL)));

                // If it's just a number and a semicolon, you can load it directly
                if (peek_token(source).id == TOK_SEMICOLON) {
                    emit_byte(chunk, OP_CONST_STORE);
                    emit_number(chunk, index);
                }
                // Otherwise you have to evaluate it
                else {
                    emit_number(chunk, index);
                    EXPR(PREC_NONE);
                    emit_byte(chunk, OP_STORE);
                }
            } else {
                EXPR(PREC_NONE);
                emit_byte(chunk, OP_STORE);
            }
        } else {
            emit_byte(chunk, OP_LOAD);
            emit_value(chunk, string_val(delexify(var.lex)));
        }
    } else {
        parse_addition(chunk, source);
    }
}

static void expression(Chunk *chunk, Source *source, Prec prec) {
    switch (prec) {
        case PREC_NONE:
        case PREC_ASSIGN:
            parse_assignment(chunk, source);
            break;
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

static bool printstmt(Chunk *chunk, Source *source) {
    if (match_token(source, TOK_PRINT)) {
        EXPR(PREC_NONE);
        if (!match_token(source, TOK_SEMICOLON))
            ERROR("Expected semicolon after print statement", -4);
        emit_byte(chunk, OP_PRINT);
        return true;
    }
    return false;
}

static void exprstmt(Chunk *chunk, Source *source) {
    expression(chunk, source, PREC_NONE);
    if (!match_token(source, TOK_SEMICOLON))
        ERROR("Expected semicolon after expression", -4);
    emit_byte(chunk, OP_POP_TOP);
}

static void statement(Chunk *chunk, Source *source) {
    if (printstmt(chunk, source))
        return;
    exprstmt(chunk, source);
}

// Probably should make it take a Source object, but hey
Chunk compile(const char *code) {
    Source source = init_source(code);

    Chunk chunk = empty_chunk();
    while (peek_token(&source).id != TOK_EOF)
        statement(&chunk, &source);
    return chunk;
}
