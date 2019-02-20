#include "chunk.h"

// precalculated. nano-optimization
#define SIG_BITS 7
#define END_FLAG 128

Chunk empty_chunk(void) {
    Chunk chunk;
    chunk.code = NULL;
    chunk.data = NULL;
    return chunk;
}

void free_chunk(Chunk *chunk) {
    vector_free(chunk->code);
    vector_free(chunk->data);
}

void emit_byte(Chunk *chunk, uint8_t byte) {
    vector_push_back(chunk->code, byte);
}

// Write's a new value and returns it's index
size_t write_value(Chunk *chunk, Value value) {
    size_t index = vector_size(chunk->data);
    vector_push_back(chunk->data, value);
    return index;
}

// Note that this increments the given IP 1 past the end of the number
unsigned long extract_number(uint8_t **ip) {
    /* The process is as follows:

    while end_flag_not_set
        byte = next_byte()
        number <<= 7
        number += byte
    number <<= 7
    number += next_byte()-128 // strip the flag's value
    */
    unsigned long number = 0;
    for (; **ip < END_FLAG; number += *((*ip)++))
        number <<= SIG_BITS;
    // What a mess!
    return (number<<SIG_BITS) + (*(*ip)++)-END_FLAG;
}

#define LAST_BITS(n) ((n)&(END_FLAG-1))
#define DROP_BITS(n) ((n)>>=SIG_BITS)
void emit_number(Chunk *chunk, unsigned long number) {
    /*
    Pretty simple. Sadly 0 is an edge case so handle that right away.
    
    So, I'm masking out the last seven bits with 0b1111111 (or 0x7F or (1<<7)-1) and pushing them to a list of
    all bytes. Then I'm shifting the number right seven times to get rid of the number masked away and added.
    
    This translates numbers into chunks of seven, so the number 131 turns into 3 and 1.
    Binary for 131:
        10000011
    Binary for 1:
        0000001
    Binary for 3:
        0000011
    Concatenate binary for 1 and 3, and you get:
        00000010000011
    Or, with zeros stripped:
        10000011
    Exactly the same as 131!

    Sadly, it generates the numbers backwards, and THAT's why we're adding it to a list: to iterate over it
    backwards and THEN add the numbers to the actual code.

    After we free our list, we add 128 to the last value in the array. This sets the 8th bit of the last number,
    which is why we've been splitting things into 7 bits instead of 8.

    That way we can encode arbitrary integers in any point in a byte array, and extract it without knowing it's
    size.
    */
    if (number == 0) {
        vector_push_back(chunk->code, 128);
        return;
    }
    uint8_t *tmp = NULL;
    while (number) {
        vector_push_back(tmp, LAST_BITS(number));
        DROP_BITS(number);
    }
    for (size_t index = vector_size(tmp); index;)
        emit_byte(chunk, tmp[--index]);
    vector_free(tmp);
    vector_back(chunk->code) += END_FLAG;
}
#undef LAST_BITS
#undef DROP_BITS

