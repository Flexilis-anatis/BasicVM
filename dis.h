#pragma once
#include "vm.h"

void dis(Chunk *chunk);
void dis_instr(uint8_t **ip, Chunk *chunk);
void print_stack(Scope *scope);