#ifndef LINKER_H_
#define LINKER_H_

#include <stdio.h>
#include "../datastructures/ast.h"
#include "../datastructures/ir.h"
#include "../datastructures/externals.h"


int link(IntermediateRepresentation *ir, SymbolicTable *table, Externals *ext);

#endif 