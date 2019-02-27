#include "value.h"
#include "func.h"
#include "closure.h"
#include "class.h"
#include <string.h>
#include <stdlib.h>

static char *strdup(const char *s) {
    char *d = malloc(strlen (s) + 1);
    if (d == NULL) return NULL;
    strcpy(d,s);
    return d;
}

#define DOUBLE_MAX_PRINT_SIZE 1000
// Prints 15 digits at most. Does not print trailing zeros
static void print_double(double num) {
    char *buffer = calloc(DOUBLE_MAX_PRINT_SIZE, 1);
    sprintf(buffer, "%.15f", num);
    size_t i;
    for (i = DOUBLE_MAX_PRINT_SIZE;
         i > 0 &&
             (buffer[i-1] == '0' || 
              buffer[i-1] == '\0');
         --i);

    char *new = calloc(DOUBLE_MAX_PRINT_SIZE, 1);
    size_t erase_ind = 0;
    while (i > 0) {
        ++erase_ind;
        --i;
        new[i] = buffer[i];
    }
    if (new[erase_ind-1] == '.')
        new[erase_ind-1] = '\0';
    printf("%s", new);
    free(buffer);
    free(new);
}

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
        case TYPE_FUNC:
            printf("{FUNCTION}");
            break;
        case TYPE_CLOSURE:
            printf("{CLOSURE}");
            break;
        case TYPE_CLASS: {
            printf("{CLASS '");
            Class *cls = VAL_AS(value, Class *);
            for (const char *c = cls->name.start; c != cls->name.end; ++c)
                putchar(*c);
            printf("'}");
            break;
        }
        case TYPE_INSTANCE: {
            printf("{INSTOF '");
            Class *cls = VAL_AS(value, Instance *)->cls;
            for (const char *c = cls->name.start; c != cls->name.end; ++c)
                putchar(*c);
            printf("'}");
            break;
        }
        case TYPE_NIL:
            printf("nil");
            break;
        }
    } else {
        print_double(value.d);
    }
}

bool same_object(Value v1, Value v2) {
    return v1.p == v2.p;
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
    if (IS_POINTER(val)) {
        switch (GET_TYPE(val)) {
            case TYPE_STRING:
                free(VAL_AS(val, char *));
                break;
            case TYPE_FUNC:
                free_func(VAL_AS(val, Func *));
                break;
            case TYPE_CLOSURE:
                free_closure(VAL_AS(val, Closure *));
                break;
            case TYPE_CLASS:
                free_class(VAL_AS(val, Class *));
                break;
            case TYPE_INSTANCE:
                free_instance(VAL_AS(val, Instance *));
                break;
        }
    }
}

bool value_true(Value val) {
    if (IS_POINTER(val)) {
        switch(GET_TYPE(val)) {
        case TYPE_STRING:
            return *VAL_AS(val, const char *) != '\0';
        case TYPE_NIL:
        case TYPE_FALSE:
            return false;
        default:
            return true;
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
    v.p = 0;
    SET_VALUE(v, string);
    SET_TYPE(v, TYPE_STRING);
    return v;
}

Value copy_val(Value source) {
    if (IS_POINTER(source)) {
        switch (GET_TYPE(source)) {
            case TYPE_STRING:
                return string_val(strdup(VAL_AS(source, const char *)));
            case TYPE_FUNC: {
                Func *copy = copy_func(VAL_AS(source, Func *));
                source.p = 0;
                SET_TYPE(source, TYPE_FUNC);
                SET_VALUE(source, copy);
                return source;
            }
            case TYPE_CLOSURE: {
                Closure *copy = copy_closure(VAL_AS(source, Closure *));
                source.p = 0;
                SET_TYPE(source, TYPE_CLOSURE);
                SET_VALUE(source, copy);
                return source;
            }
            case TYPE_CLASS: {
                Class *copy = copy_class(VAL_AS(source, Class *));
                source.p = 0;
                SET_TYPE(source, TYPE_CLASS);
                SET_VALUE(source, copy);
                return source;
            }
            case TYPE_INSTANCE: {
                Instance *copy = copy_instance(VAL_AS(source, Instance *));
                source.p = 0;
                SET_TYPE(source, TYPE_INSTANCE);
                SET_VALUE(source, copy);
                return source;
            }
            default:
                break;
        }
    }
    return source;
}

Value nil_val() {
    Value v;
    v.p = 0;
    SET_TYPE(v, TYPE_NIL);
    return v;
}