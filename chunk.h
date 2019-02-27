#pragma once
#include <stdint.h>
#include <math.h>
#include "vec/vector.h"
#include "value.h"
#include "token.h"

typedef enum {
    OP_PUSH, OP_NEG, OP_NOT, OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD, OP_PRINT,
    OP_RETURN, OP_POP_TOP, OP_CONST_JMP, OP_COND_JMP, OP_LOAD, OP_STORE,
    OP_CONST_STORE, OP_PUTS, OP_CALL, OP_BIND, OP_LT, OP_LTE, OP_GT, OP_GTE,
    OP_EQU, OP_IS, OP_AND, OP_OR, OP_NEW
} OpCode;

typedef struct {
    vector(Lex) idents;
    vector(Value) data;
    vector(long) jumps;
} Consts;

// A chunk of code
typedef struct {
    vector(uint8_t) code;
    Consts *consts;
} Chunk;

#define BITS(n) (sizeof(unsigned long)*8 - __builtin_clzl((n) | 1))
#define BYTES(n) ((n + ((7 - (n % 7)) % 7)) / 7)

// Initializes code to NULL
Chunk empty_chunk(Consts *consts);
// Initilizes consts to NULL
Consts *empty_consts(void);
// Read the name
void free_chunk(Chunk *chunk);
// Free's jump table, identifier table, and constants
void free_consts(Consts *consts);

// Write a byte of data to the chunk
void emit_byte(Chunk *chunk, uint8_t byte);

// Add a value to the data vector. Returns it's index
size_t write_value(Chunk *chunk, Value value);

// Pass in a pointer to an IP. It will increment it past the next number and return the numbers value
unsigned long extract_number(uint8_t **ip);
// Write's a number to the chunk
void emit_number(Chunk *chunk, unsigned long number);

size_t jump_ind(Chunk *chunk, long jump_size);
void emit_constant(Chunk *chunk, Value val);
void emit_ident(Chunk *chunk, Lex ident);