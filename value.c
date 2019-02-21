#include "value.h"
#include <string.h>
#include <stdlib.h>

/*#define INT_TYPE uint64_t
void printbits(INT_TYPE v) {
    char buf[sizeof(INT_TYPE)*8+1];
    size_t i = sizeof(INT_TYPE)*8+1;
    for (; i--; v >>= 1) buf[i]='0' + (v & 1);
    buf[65] = '\0';
    printf(buf);
}*/

void print_value(Value value) {
    if (IS_POINTER(value)) {
        switch (GET_TYPE(value)) {
        case TYPE_STRING:
            printf("%s", VAL_AS(value, const char *));
            break;
        case TYPE_TRUE:
            printf("true");
            break;
        case TYPE_FALSE:
            printf("false");
            break;
        }
    } else {
        printf("%f", value.d);
    }
}

bool same_object(Value v1, Value v2) {
    return v1.d == v2.d;
}

bool same_value(Value v1, Value v2) {
    if (GET_TYPE(v1) != GET_TYPE(v2))
        return false;

    if (IS_POINTER(v1)) {
        switch(GET_TYPE(v1)) {
        case TYPE_STRING:
            return strcmp(VAL_AS(v1, const char *), VAL_AS(v2, const char *)) == 0;
        }
    }
    return same_object(v1, v2);
}

Value double_val(double val) {
    Value v;
    v.d = val;
    return v;
}

void free_value(Value val) {
    if (IS_POINTER(val))
        free(VAL_AS(val, void *)); // naive for now
}

bool value_true(Value val) {
    if (IS_POINTER(val)) {
        switch(GET_TYPE(val)) {
        case TYPE_STRING:
            return *VAL_AS(val, const char *) != '\0';
        case TYPE_TRUE:
            return true;
        default:
            return false;
        }
    } else {
        return val.d != 0;
    }
}

Value bool_value(bool cond) {
    Value v;
    v.p = 0;
    SET_TYPE(v, cond ? TYPE_TRUE : TYPE_FALSE);
    return v;
}

Value string_val(char *string) {
    Value v;
    SET_VALUE(v, string);
    SET_TYPE(v, TYPE_STRING);
    return v;
}