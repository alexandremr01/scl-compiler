#ifndef RISCV_H_
#define RISCV_H_

#include "../register_mapping.h"
#include "../ir.h"

void printIR(IntermediateRepresentation *ir, FILE *f_asm, FILE *f_bin, RegisterMapping *rm);
#endif 
