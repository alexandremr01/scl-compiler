#ifndef RISCV_H_
#define RISCV_H_

#include "../backend/register_mapping.h"
#include "../datastructures/ir.h"

void wirteIR(IntermediateRepresentation *ir, FILE *f_asm, FILE *f_bin, RegisterMapping *rm, int includeASMComments);
#endif 
