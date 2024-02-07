#ifndef CODEGEN_H_
#define CODEGEN_H_

#include <stdio.h>
#include "ast.h"
#include "ir.h"

IntermediateRepresentation *codeGen(AbstractSyntaxTree *tree);

#endif 