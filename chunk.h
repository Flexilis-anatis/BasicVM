#pragma once
#include <stdint.h>
#include "vec/vector.h"
#include "value.h"

typedef enum {
    OP_PUSH, OP_NEG, OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD, OP_PRINT, OP_RETURN,
    OP_POP_TOP, OP_CONST_JMP, OP_COND_JMP, OP_LOAD, OP_STORE, OP_CONST_STORE,
    OP_PUTS, 
} OpCode;

// A chunk of code
typedef struct {
    vector(uint8_t) code;
    vector(Value) data;
} Chunk;

// Initializes vectors to NULL
Chunk empty_chunk(void);
// Read the name
void free_chunk(Chunk *chunk);

// Write a byte of data to the chunk
void emit_byte(Chunk *chunk, uint8_t byte);

// Emit the index of a new value immediatly
#define emit_value(chunk, value) emit_number((chunk), write_value((chunk), (value)))
// Add a value to the data vector. Returns it's index
size_t write_value(Chunk *chunk, Value value);

// Pass in a pointer to an IP. It will increment it past the next number and return the numbers value
unsigned long extract_number(uint8_t **ip);
// Write's a number to the chunk
void emit_number(Chunk *chunk, unsigned long number);