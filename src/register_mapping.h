#ifndef REGISTER_MAPPING_H_
#define REGISTER_MAPPING_H_

#include <stdio.h>
#include "ir.h"

typedef struct registerMapping {
    char **map;
} RegisterMapping;

RegisterMapping *newRegisterMapping(IntermediateRepresentation *ir);
char *getRegister(RegisterMapping *rm, int temporary);

#endif 