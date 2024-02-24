#ifndef RISCV_H_
#define RISCV_H_

#include "../backend/register_assignment.h"
#include "../datastructures/ir.h"
#include "../datastructures/objectcode.h"

ObjectCode *translateIRToObj(IntermediateRepresentation *ir, RegisterAssignment *rm, int includeASMComments, int asmDialect);

#endif 
