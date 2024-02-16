#ifndef RISCV_H_
#define RISCV_H_

#include "../backend/register_assignment.h"
#include "../datastructures/ir.h"

void wirteIR(IntermediateRepresentation *ir, FILE *f_asm, FILE *f_bin, RegisterAssignment *rm, int includeASMComments);
#endif 
