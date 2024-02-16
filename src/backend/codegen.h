#ifndef CODEGEN_H_
#define CODEGEN_H_

#include <stdio.h>
#include "../datastructures/ast.h"
#include "../datastructures/ir.h"


IntermediateRepresentation *codeGen(AbstractSyntaxTree *tree);
void call(IntermediateRepresentation *ir, SymbolicTableEntry *fn);

#endif 