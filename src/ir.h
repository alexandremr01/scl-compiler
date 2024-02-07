#ifndef IR_H_
#define IR_H_

// Intermediate Representation

#include <stdlib.h>
#include "symbolic_table.h"
#include "datatypes.h"

typedef enum sourceKind {
    CONSTANT_SOURCE, REG_SOURCE
} SourceKind;

typedef enum instruction {
    MOV, LOAD, STORE
} Instruction;

typedef struct irNode {
    struct irNode *next;
    Instruction instruction;
    int dest;
    SourceKind source_kind;
    int source;
} IRNode;

typedef struct intermediateRepresentation {
    struct irNode *head;
    struct irNode *tail;
} IntermediateRepresentation;

IntermediateRepresentation *newIntermediateRepresentation();
IRNode *newIRNode();
void addMovIR(IntermediateRepresentation *ir, int destination);
void addNode(IntermediateRepresentation *ir, IRNode *node);
void printIR(IntermediateRepresentation *ir, FILE *f);

#endif