#include "dis.h"
#include <stdio.h>

// Format is (@byte_offset) padded_instruct_number: instruct_names args? (arg_vals)?
// example: (@41) 0032: PUSH 13 (3.213000)
// 41st byte, 32nd instruction, pushes the value 3.213 onto the stack

/**
 * print - Print a string and a new line
 * @param string - the string to print
 * @returns void
 */
void print(const char *string) {
    const char *c = string;
    do
        putchar(*c);
    while (*c++);
    putchar('\n');
}

/**
 * @brief data_op - Print the assembly for an instruction that has args
 * @param name - the name of the instruction
 * @param ip - a pointer to an instruction pointer. Will be incremented past data
 * @param chunk - the current chunk of data
 * @param args - the amount of arguments to the instruction
 * @returns void
 */
void data_op(const char *name, uint8_t **ip, Chunk *chunk, size_t args) {
    printf(name);
    putchar(' ');
    vector(unsigned long) nums = NULL;
    while (args--) {
        unsigned long num = extract_number(ip);
        (*ip)--;
        vector_push_back(nums, num);
        printf("%lu ", num);
    }
    putchar('(');
    for (size_t i = 0; i < vector_size(nums)-1; ++i) {
        print_value(chunk->data[nums[i]]);
        printf(", ");
    }
    if (vector_size(nums))
        print_value(chunk->data[*(vector_end(nums)-1)]);
    printf(")\n");
    vector_free(nums);
}

/**
 * @brief print_data - print the constants of a chunk
 * @param chunk - the chunk to print the data of
 * @returns void
 */
void print_data(Chunk *chunk) {
    putchar('[');
    // Don't want a trailing comma, but don't want to segfault on empty constant pools either (VERY unlikely)
    if (vector_size(chunk->data)) {
        for (size_t i = 0; i < vector_size(chunk->data)-1; ++i) {
            printf("(%lu) ", i);
            print_value(chunk->data[i]);
            printf(", ");
        }
        printf("(%lu) ", vector_size(chunk->data)-1);
        print_value(*(vector_end(chunk->data)-1));
    }
    printf("]\n");
}

/**
 * @brief dis - disassemble a chunk of bytecode
 * @param chunk - the chunk to disassemble
 * @returns void
 */
void dis(Chunk *chunk) {
    // Print the constants and a seperator
    print_data(chunk);
    printf("--------------------------\n");

    size_t ind = 0;
    for(uint8_t *ip = chunk->code; ip < vector_end(chunk->code); ++ip, ++ind) {
        printf("(@%lu) %lu: ", ind, ip-chunk->code);

        switch (*ip) {
            case OP_PUSH:
                data_op("PUSH", &ip, chunk, 1);
                break;
            case OP_NEG:
                print("NEG");
                break;
            case OP_ADD:
                print("ADD");
                break;
            case OP_SUB:
                print("SUB");
                break;
            case OP_DIV:
                print("DIV");
                break;
            case OP_MUL:
                print("MUL");
                break;
            case OP_MOD:
                print("MOD");
                break;
            case OP_RETURN:
                print("RET");
                break;
            default:
                printf("UNKNOWN: %u", *ip);
                break;
        }
    }
}