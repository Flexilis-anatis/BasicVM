#pragma once
#include "vm.h"
#include <stdbool.h>

void run_vm(VM *vm);
void run_scope(Scope *scope);
bool run_next(Scope *scope);