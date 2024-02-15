#ifndef SEMANTIC_ANALYSIS_H_
#define SEMANTIC_ANALYSIS_H_

#include <stdio.h>
#include "ast.h"

int semanticAnalysis(AbstractSyntaxTree *tree, SymbolicTable* symbolicTable, int debug);


#endif 