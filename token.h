#pragma once

typedef enum {
    TOK_NUMBER, TOK_ADD, TOK_MUL, TOK_DIV, TOK_SUB, TOK_MOD, TOK_EOF,
    TOK_LPAREN, TOK_RPAREN, TOK_IDENT, TOK_ASSIGN, TOK_PRINT, TOK_SEMICOLON
} TokID;

typedef struct {
    const char *start, *end;
} Lex;

typedef struct {
    TokID id;
    Lex lex;
} Token;

