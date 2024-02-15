#ifndef REGISTER_MAPPING_H_
#define REGISTER_MAPPING_H_

#include <stdio.h>
#include "../ir.h"

typedef struct registerMapping {
    int *map;
} RegisterMapping;

RegisterMapping *newRegisterMapping(IntermediateRepresentation *ir);
int getRegisterAssignment(RegisterMapping *rm, int temporary);
void freeRegisterMapping(RegisterMapping *rm);

#endif 