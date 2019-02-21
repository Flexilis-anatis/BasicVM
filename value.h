#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/* Double Floating point storage:
 * Sign bit (1)                      Mantissa (52)
 *    - ----------- ----------------------------------------------------
 *        Exp (11)
 * When all 11 exponent bits are set, it is considered NaN. This gives us 52 Mantissa bits and a sign bit to
 * play with. Since most 64-bit platforms only actually use 48-bit addresses, we can store a pointer and a few
 * nice flags!
 *
 * I'm storing the four extra Mantissa bits as the types, giving me 16 types (and I can do some tricks with
 * the sign bit to get 32 later if I need)
 * 
 * Another option for singleton values is to assign all of them one value in the first four Mantissa bits, and
 * then use the rest of the Mantissa bits to tell apart true, false, null, etc.
 *     if ((type = GET_TYPE(value)) == TYPE_SINGLETON) type = GET_VALUE(value)
 * Would do it!
 */

#define POINTER_MASK 281474976710655U // (1 << 52) - 1
#define NAN_MASK 9218868437227405312U // (1 << 63) through (1 << 53)
#define SIGN_MASK 18446744073709551616U // 1 << 63
#define TYPE_MASK 4222124650659840U // (1 << 53) through (1 << 49)
#define MANTISSA_MASK 4503599627370495U // SIGN_MASK | POINTER_MASK

#define VAL_AS(vl,type) ((type)(vl.p&POINTER_MASK))
#define IS_POINTER(vl) (((vl).p&NAN_MASK)==NAN_MASK)
#define SET_VALUE(vl,to) ((vl).p=((uint64_t)(to))|NAN_MASK)
#define GET_VALUE(vl) (((vl).p)&POINTER_MASK)
#define GET_MANTISSA(vl) ((vl).p&MANTISSA_MASK)
#define GET_TYPE(vl) (((vl).p&TYPE_MASK)>>48)
#define SET_TYPE(vl,to) ((vl).p=((vl.p)&~TYPE_MASK)|(((uint64_t)(to))<<48)|NAN_MASK)

typedef enum {
    TYPE_TRUE, TYPE_FALSE, TYPE_STRING, 
} Type;

typedef union {
    double d;
    uint64_t p;
} Value;

typedef struct {
    const char *start, *end;
} String;

void print_value(Value value);
bool same_object(Value v1, Value v2); // this is only true if they actually point to the same memory location
bool same_value(Value v1, Value v2); // this is true if the values and types are both the same
Value double_val(double val);
void free_value(Value value);
bool value_true(Value value);
Value bool_value(bool cond);
Value string_val(const char *string);