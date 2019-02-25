#pragma once
#include "vm.h"

typedef enum {
    INTERP_OK, INTERP_END, INTERP_RET
} InterpStat;

void run_vm(VM *vm);
InterpStat run_scope(Scope *scope);
InterpStat run_next(Scope *scope);