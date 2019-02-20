#include "dis.h"
#include <stdio.h>

// Format is (@byte_offset) padded_instruct_number: instruct_names args? (arg_vals)?
// example: (@41) 0032: PUSH 13 (3.213000)
// 41st byte, 32nd instruction, pushes the value 3.213 onto the stack

/**
 * @brief simple_op - Print the assembly for a simple instruction
 * @param padding - the amount of digits to pad the index
 * @param ind - the index the instruction is
 * @param name - the name of the instruction
 * @returns void
 */
void simple_op(int padding, size_t ind, const char *name) {
    printf("%0*lu: %s\n", padding, ind, name);
}

/**
 * @brief data_op - Print the assembly for an instruction that has args
 * @param padding - the amount of digits to pad the index
 * @param ind - the index the instruction is
 * @param ip - a pointer to an instruction pointer. Will be incremented past data
 * @param chunk - the current chunk of data
 * @param name - the name of the instruction
 * @param args - the amount of arguments to the instruction
 * @returns void
 */
void data_op(int padding, size_t ind, uint8_t **ip, Chunk *chunk, const char *name, size_t args) {
    printf("%0*lu: %s ", padding, ind, name);
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

    // dirty hack to get the max amount of digits in an instruct index. can be greater than neccessary
    // with lots of instructs that take args
    size_t digits, size = vector_size(chunk->data);
    for(digits = 1; size /= 10; digits++);

    size_t ind = 0;
    for(uint8_t *ip = chunk->code; ip < vector_end(chunk->code); ++ip, ++ind) {
        // print the memory offset
        printf("(@%lu) ", ip-chunk->code);
        switch (*ip) {
            case OP_PUSH:
                data_op(digits, ind, &ip, chunk, "PUSH", 1);
                break;
            case OP_NEG:
                simple_op(digits, ind, "NEG");
                break;
            case OP_ADD:
                simple_op(digits, ind, "ADD");
                break;
            case OP_SUB:
                simple_op(digits, ind, "SUB");
                break;
            case OP_DIV:
                simple_op(digits, ind, "DIV");
                break;
            case OP_MUL:
                simple_op(digits, ind, "MUL");
                break;
            case OP_MOD:
                simple_op(digits, ind, "MOD");
                break;
            case OP_RETURN:
                simple_op(digits, ind, "RET");
                break;
            default:
                printf("UNKNOWN: %u", *ip);
                break;
        }
    }
}