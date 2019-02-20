#pragma once

typedef enum {
    TOK_NUMBER, TOK_ADD, TOK_MUL, TOK_DIV, TOK_SUB, TOK_MOD, TOK_EOF,
    TOK_LPAREN, TOK_RPAREN,
} TokID;

typedef struct {
    TokID id;
    struct {
        const char *start, *end;
    } lex;
} Token;

