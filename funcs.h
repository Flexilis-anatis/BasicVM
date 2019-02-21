#pragma once
#include "vm.h"
Value last_val(Scope *);
void push_val(Scope *, Value);
Value get_val(Scope *);
void pop_back(Scope *);
Value pop(Scope *);

// defines multi-line functions for the VM 

// Macro abuse FTW
#define F(n) void op_##n(Scope *scope)
F(add);
F(sub);
F(mul);
F(div);
F(mod);
F(print);
F(puts);
F(const_jmp);
F(cond_jmp);
F(const_store);
F(store);
F(load);
#undef F