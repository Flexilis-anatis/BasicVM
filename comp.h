#pragma once
#include "scanner.h"
#include "chunk.h"

// Swapping the order around messes with precedence, but the chain always stops at PREC_ATOM no matter what
// Good way to disable features, I guess
typedef enum {
    PREC_NONE,
    PREC_ASSIGN,
    PREC_CLOSURE,
    PREC_LOGIC,
    PREC_ADD,
    PREC_MUL,
    PREC_NEGATE,
    PREC_PAREN,
    PREC_ATOM
} Prec;

/*
Compile a chunk of source code
*/
Chunk compile(const char *code);