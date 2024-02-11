#ifndef IR_H_
#define IR_H_

// Intermediate Representation

#include <stdlib.h>
#include "symbolic_table.h"
#include "datatypes.h"

typedef enum sourceKind {
    CONSTANT_SOURCE, REG_SOURCE, VARIABLE_SOURCE
} SourceKind;

typedef enum instruction {
    MOV, LOAD, STORE, COMMENT, ADD, SUB
} Instruction;

typedef struct irNode {
    struct irNode *next;
    Instruction instruction;
    int dest;
    SourceKind sourceKind;

    int source;
    int source2;
    SymbolicTableEntry *varSource;

    char *comment;
} IRNode;

typedef struct symbolicTableGlobals {
    SymbolicTableEntry *entry;
    struct symbolicTableGlobals *next;
} SymbolicTableGlobals;

typedef struct intermediateRepresentation {
    struct irNode *head;
    struct irNode *tail;
    int nextTempReg;
    SymbolicTableGlobals *globals;
    int lastAddress;
} IntermediateRepresentation;

IntermediateRepresentation *newIntermediateRepresentation();
void freeIntermediateRepresentation(IntermediateRepresentation *ir);
void addMovIR(IntermediateRepresentation *ir, int destination);
void addNode(IntermediateRepresentation *ir, IRNode *node);
void addLoadImIR(IntermediateRepresentation *ir, int destination, int value);
void addLoadAddressIR(IntermediateRepresentation *ir, int destination, SymbolicTableEntry *stEntry);
void addAdditionIR(IntermediateRepresentation *ir, int src1, int src2, int destination);
void addSubtractionIR(IntermediateRepresentation *ir, int src1, int src2, int destination);
void addStoreIR(IntermediateRepresentation *ir, int destination_address, int shift, int register_source);
void addLoadMemIR(IntermediateRepresentation *ir, int destination_address, int shift, int register_source);
void addCommentIR(IntermediateRepresentation *ir, char *comment);

#endif