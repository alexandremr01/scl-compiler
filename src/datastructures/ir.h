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

#define T0_REGISTER -12
#define T1_REGISTER -11
#define T2_REGISTER -10
#define T3_REGISTER -9
#define T4_REGISTER -8
#define T5_REGISTER -7
#define T6_REGISTER -6

int get_tx_register(int x);

typedef enum sourceKind {
    CONSTANT_SOURCE, REG_SOURCE, VARIABLE_SOURCE
} SourceKind;

typedef enum instruction {
    MOV, LOAD, STORE, COMMENT, ADD, SUB, LABEL, JUMP, JUMP_REGISTER, NOP,
    BEQ, BNEQ, BLE, BLT, BGE, BGQ, BGT, MUL, AUIPC, RAW, RELATIVE_JUMP, NEG, DATA
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

    struct irNode *target;

    char *comment;
    int address;
} IRNode;

typedef struct intermediateRepresentation {
    struct irNode *head;
    struct irNode *tail;
    int nextTempReg;
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
void addMultiplicationIR(IntermediateRepresentation *ir, int src1, int src2, int destination);
void addSubtractionIR(IntermediateRepresentation *ir, int src1, int src2, int destination);
void addStoreIR(IntermediateRepresentation *ir, int destination_address, int shift, int register_source);
void addLoadMemIR(IntermediateRepresentation *ir, int destination_address, int shift, int register_source);
void addCommentIR(IntermediateRepresentation *ir, char *comment);
void addLoadVarAddress(IntermediateRepresentation *ir, int destinationRegister, int sourceRegister, SymbolicTableEntry *entry);
void addGetPC(IntermediateRepresentation *ir, int destinationRegister, int imm);
void addStoreVarAddress(IntermediateRepresentation *ir, int destinationRegister, int sourceRegister, SymbolicTableEntry *entry);
void addGetPCVarAddress(IntermediateRepresentation *ir, int destinationRegister, SymbolicTableEntry *entry);

IRNode* addDataIR(IntermediateRepresentation *ir, int data);
void addLabelIR(IntermediateRepresentation *ir, SymbolicTableEntry *entry); 
void addJumpIR(IntermediateRepresentation *ir, SymbolicTableEntry *entry); 
void addJumpRegisterIR(IntermediateRepresentation *ir, int destinationRegister);
void addRawIR(IntermediateRepresentation *ir, char *instruction);

void addAdditionImIR(IntermediateRepresentation *ir, int dest, int src, int imm);
void addNegIR(IntermediateRepresentation *ir, int dest, int src);
IRNode *addJumpImIR(IntermediateRepresentation *ir, int imm);
void addNopIR(IntermediateRepresentation *ir);
void addRelativeJump(IntermediateRepresentation *ir, IRNode *node);

IRNode* addBEQIR(IntermediateRepresentation *ir, int src1, int src2, int destination);
IRNode* addBNEQIR(IntermediateRepresentation *ir, int src1, int src2, int destination);
IRNode* addBLEIR(IntermediateRepresentation *ir, int src1, int src2, int destination);
IRNode* addBGEIR(IntermediateRepresentation *ir, int src1, int src2, int destination);
IRNode* addBLTIR(IntermediateRepresentation *ir, int src1, int src2, int destination);
IRNode* addBGTIR(IntermediateRepresentation *ir, int src1, int src2, int destination);
                                                                                               
#endif