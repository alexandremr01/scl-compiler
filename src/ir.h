#ifndef IR_H_
#define IR_H_

// Intermediate Representation

#include <stdlib.h>
#include "symbolic_table.h"
#include "datatypes.h"

#define SP_REGISTER -1
#define A0_REGISTER -2
#define X0_REGISTER -3
#define FP_REGISTER -4
#define RA_REGISTER -5

typedef enum sourceKind {
    CONSTANT_SOURCE, REG_SOURCE, VARIABLE_SOURCE
} SourceKind;

typedef enum instruction {
    MOV, LOAD, STORE, COMMENT, ADD, SUB, LABEL, JUMP, JUMP_REGISTER, NOP, BEQ, BNEQ
} Instruction;

typedef struct irNode {
    struct irNode *next;
    Instruction instruction;
    int dest;
    SourceKind sourceKind;

    int source;
    int source2;
    int imm;
    SymbolicTableEntry *varSource;

    char *comment;
    int address;
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
    int lastStackAddress;
} IntermediateRepresentation;

IntermediateRepresentation *newIntermediateRepresentation();

IRNode *newIRNode(Instruction instruction);
void addNode(IntermediateRepresentation *ir, IRNode *node);

void freeIntermediateRepresentation(IntermediateRepresentation *ir);
void addMovIR(IntermediateRepresentation *ir, int destination, int source);
void addNode(IntermediateRepresentation *ir, IRNode *node);
void addLoadImIR(IntermediateRepresentation *ir, int destination, int value);
void addLoadAddressIR(IntermediateRepresentation *ir, int destination, SymbolicTableEntry *stEntry);
void addAdditionIR(IntermediateRepresentation *ir, int src1, int src2, int destination);
void addSubtractionIR(IntermediateRepresentation *ir, int src1, int src2, int destination);
void addStoreIR(IntermediateRepresentation *ir, int destination_address, int shift, int register_source);
void addLoadMemIR(IntermediateRepresentation *ir, int destination_address, int shift, int register_source);
void addCommentIR(IntermediateRepresentation *ir, char *comment);

void addLabelIR(IntermediateRepresentation *ir, SymbolicTableEntry *entry); 
void addJumpIR(IntermediateRepresentation *ir, SymbolicTableEntry *entry); 
void addJumpRegisterIR(IntermediateRepresentation *ir, int destinationRegister);

void addAdditionImIR(IntermediateRepresentation *ir, int dest, int src, int imm);
IRNode *addJumpImIR(IntermediateRepresentation *ir, int imm);
void addNopIR(IntermediateRepresentation *ir);

void addBEQIR(IntermediateRepresentation *ir, int src1, int src2, int destination);
IRNode* addBNEQIR(IntermediateRepresentation *ir, int src1, int src2, int destination);

void addLTIR(IntermediateRepresentation *ir, int src1, int src2, int destination);
void addGTIR(IntermediateRepresentation *ir, int src1, int src2, int destination);
void addLEQIR(IntermediateRepresentation *ir, int src1, int src2, int destination);
void addGEQIR(IntermediateRepresentation *ir, int src1, int src2, int destination);
void addEQIR(IntermediateRepresentation *ir, int src1, int src2, int destination);
void addDiffIR(IntermediateRepresentation *ir, int src1, int src2, int destination);
                                                                                               
#endif