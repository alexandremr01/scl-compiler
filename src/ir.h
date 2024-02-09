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
    MOV, LOAD, STORE, COMMENT
} Instruction;

typedef struct irNode {
    struct irNode *next;
    Instruction instruction;
    int dest;
    SourceKind sourceKind;
    int source;
    char *comment;
} IRNode;

typedef struct intermediateRepresentation {
    struct irNode *head;
    struct irNode *tail;
    int nextTempReg;
} IntermediateRepresentation;

IntermediateRepresentation *newIntermediateRepresentation();
IRNode *newIRNode();
void addMovIR(IntermediateRepresentation *ir, int destination);
void addNode(IntermediateRepresentation *ir, IRNode *node);
void addLoadImIR(IntermediateRepresentation *ir, int destination, int value);
void addStoreIR(IntermediateRepresentation *ir, int destination_address, int shift, int register_source);
void addLoadMemIR(IntermediateRepresentation *ir, int destination_address, int shift, int register_source);
void addCommentIR(IntermediateRepresentation *ir, char *comment);

#endif