#ifndef IR_H_
#define IR_H_

// Intermediate Representation

#include <stdlib.h>
#include "symbolic_table.h"
#include "datatypes.h"

#define FS3_REGISTER -23

#define SP_REGISTER -1
#define A0_REGISTER -2
#define FA0_REGISTER -15
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
    MOV, LOAD, STORE, COMMENT, ADD, SUB, LABEL, JUMP, JUMP_REGISTER, NOP, LUI,
    BEQ, BNEQ, BLE, BLT, BGE, BGQ, BGT, MUL, AUIPC, RAW, RELATIVE_JUMP, NEG, DATA,
    FMVWX, CSRRW, FMOV, FLT, FLE, FSGNJ, FSGNJN,
    SETPA, SETPB, MACC, MACCSTORE
} Instruction;

typedef struct irNode {
    struct irNode *next;
    Instruction instruction;
    int dest;
    SourceKind sourceKind;

    int source;
    int source2;
    int imm;
    int isFloat;
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
    int *temporaryIsFloat;
    int _maxTemporaries;
} IntermediateRepresentation;

IntermediateRepresentation *newIntermediateRepresentation();
int registerNewTemporary(IntermediateRepresentation *ir, int isFloat);

IRNode *newIRNode(Instruction instruction);
void addNode(IntermediateRepresentation *ir, IRNode *node);

void freeIntermediateRepresentation(IntermediateRepresentation *ir);
void addMovIR(IntermediateRepresentation *ir, int destination, int source);
void addFloatMovIR(IntermediateRepresentation *ir, int destination, int source);
void addMovFromIntegerToFloat(IntermediateRepresentation *ir, int destination, int source);
void addNode(IntermediateRepresentation *ir, IRNode *node);
void addLoadImIR(IntermediateRepresentation *ir, int destination, int value);
void addCSRReadWrite(IntermediateRepresentation *ir, int destination, int value, int source);
void addLoadUpperImIR(IntermediateRepresentation *ir, int destination, int value);
void addLoadAddressIR(IntermediateRepresentation *ir, int destination, SymbolicTableEntry *stEntry);
void addAdditionIR(IntermediateRepresentation *ir, int src1, int src2, int destination, int isFloat);
void addMultiplicationIR(IntermediateRepresentation *ir, int src1, int src2, int destination, int isFloat);
void addSubtractionIR(IntermediateRepresentation *ir, int src1, int src2, int destination, int isFloat);
void addStoreIR(IntermediateRepresentation *ir, int destination_address, int shift, int register_source, int isFloat);
void addLoadMemIR(IntermediateRepresentation *ir, int destination_address, int shift, int register_source, int isFloat);
void addCommentIR(IntermediateRepresentation *ir, char *comment);
void addLoadVarAddress(IntermediateRepresentation *ir, int destinationRegister, int sourceRegister, SymbolicTableEntry *entry, int isFloat);
void addGetPC(IntermediateRepresentation *ir, int destinationRegister, int imm);
void addStoreVarAddress(IntermediateRepresentation *ir, int destinationRegister, int sourceRegister, SymbolicTableEntry *entry, int isFloat);
IRNode* addGetPCVarAddress(IntermediateRepresentation *ir, int destinationRegister, SymbolicTableEntry *entry);
void addSumAddressDistance(IntermediateRepresentation *ir, int destinationRegister, SymbolicTableEntry *entry, int correction);

IRNode* addDataIR(IntermediateRepresentation *ir, int data);
void addLabelIR(IntermediateRepresentation *ir, SymbolicTableEntry *entry); 
void addJumpIR(IntermediateRepresentation *ir, SymbolicTableEntry *entry); 
void addJumpRegisterIR(IntermediateRepresentation *ir, int destinationRegister);
void addRawIR(IntermediateRepresentation *ir, char *instruction);

IRNode* addSetPA(IntermediateRepresentation *ir, int address);
void addSetPB(IntermediateRepresentation *ir, int address);
IRNode* addMACC(IntermediateRepresentation *ir);
void addMACCStore(IntermediateRepresentation *ir, int address);

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

IRNode* addFloatLT(IntermediateRepresentation *ir, int src1, int src2, int destination);
IRNode* addFloatGT(IntermediateRepresentation *ir, int src1, int src2, int destination);
IRNode* addFloatLEQ(IntermediateRepresentation *ir, int src1, int src2, int destination);
IRNode* addFloatGEQ(IntermediateRepresentation *ir, int src1, int src2, int destination);

IRNode* addFSGNJ(IntermediateRepresentation *ir, int src1, int src2, int destination);
IRNode* addFSGNJN(IntermediateRepresentation *ir, int src1, int src2, int destination);


#endif