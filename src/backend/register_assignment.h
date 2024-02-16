#ifndef REGISTER_ASSIGNMENT_H_
#define REGISTER_ASSIGNMENT_H_

#include <stdio.h>
#include "../datastructures/ir.h"

typedef struct RegisterAssignment {
    int *map;
} RegisterAssignment;

RegisterAssignment *newRegisterAssignment(IntermediateRepresentation *ir);
int getRegisterAssignment(RegisterAssignment *rm, int temporary);
void freeRegisterAssignment(RegisterAssignment *rm);

#endif 