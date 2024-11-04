#include "ir.h"
#include <stdio.h>

int get_tx_register(int x) {
    return T0_REGISTER + x;
}

void printIR(IntermediateRepresentation *ir) {
    IRNode *p = ir->head;
    int i = 0;
    while (p != NULL) {
        printf("%d: %d\n", i, p->instruction);
        p = p->next;
        i += 1;
    }
}

IntermediateRepresentation *newIntermediateRepresentation(){
    IntermediateRepresentation *ir = (IntermediateRepresentation *) malloc(sizeof(IntermediateRepresentation));
    ir->head = NULL;
    ir->tail = NULL;
    ir->nextTempReg = 0;
    ir->lastAddress = 0;
    ir->lastStackAddress = 0;
    ir->_maxTemporaries = 256;
    ir->temporaryIsFloat = (int *) malloc(sizeof(int) * ir->_maxTemporaries);
    return ir;
}

int registerNewTemporary(IntermediateRepresentation *ir, int isFloat){
    int id = ir->nextTempReg++;
    ir->temporaryIsFloat[id] = isFloat;
    if (id == ir->_maxTemporaries - 2) {
        ir->_maxTemporaries *= 2;
        int *tmp = (int *) realloc(ir->temporaryIsFloat, sizeof(int) * ir->_maxTemporaries);
        ir->temporaryIsFloat = tmp;
    }
    return id;
}

void addNode(IntermediateRepresentation *ir, IRNode *node){
    if (ir->head==NULL) ir->head=node;
    if (ir->tail==NULL) 
        ir->tail=node;
    else {
        ir->tail->next = node;
        ir->tail = node;
    }
    if (node->instruction != COMMENT) {
        node->address = ir->lastAddress;
        ir->lastAddress += 4;
    }
}

IRNode *newIRNode(Instruction instruction){
    IRNode * node = (IRNode *) malloc(sizeof(IRNode));
    node->instruction = instruction;
    node->next = NULL;
    node->dest = X0_REGISTER;
    node->source = X0_REGISTER;
    node->source2 = X0_REGISTER;
    node->isFloat = 0;
    return node;
}

void addLoadImIR(IntermediateRepresentation *ir, int destination, int value) {
    IRNode * node = newIRNode(LOAD);
    node->dest = destination;
    node->sourceKind = CONSTANT_SOURCE;
    node->source = value;
    addNode(ir, node);
}

void addLoadUpperImIR(IntermediateRepresentation *ir, int destination, int value) {
    IRNode * node = newIRNode(LUI);
    node->dest = destination;
    node->sourceKind = CONSTANT_SOURCE;
    node->source = value;
    addNode(ir, node);
}

void addLoadAddressIR(IntermediateRepresentation *ir, int destination, SymbolicTableEntry *stEntry){
    IRNode * node = newIRNode(LOAD);
    node->dest = destination;
    node->sourceKind = VARIABLE_SOURCE;
    node->varSource = stEntry;
    addNode(ir, node);  
}

void addStoreIR(IntermediateRepresentation *ir, int destination_address, int shift, int register_source){
    IRNode * node = newIRNode(STORE);
    node->instruction = STORE;
    node->dest = destination_address;
    node->sourceKind = REG_SOURCE;
    node->source = register_source;
    node->imm = shift;
    addNode(ir, node);
}

IRNode *addDataIR(IntermediateRepresentation *ir, int data) {
    IRNode * node = newIRNode(DATA);
    node->imm = data;
    addNode(ir, node);
    return node;
}

void addLoadMemIR(IntermediateRepresentation *ir, int destination_address, int shift, int register_source){
    IRNode * node = newIRNode(LOAD);
    node->dest = destination_address;
    node->sourceKind = REG_SOURCE;
    node->source = register_source;
    node->imm = shift;
    addNode(ir, node);
}

void addCommentIR(IntermediateRepresentation *ir, char *comment){
    IRNode * node = newIRNode(COMMENT);
    node->comment = comment;
    addNode(ir, node);
}

void addRawIR(IntermediateRepresentation *ir, char *instruction){
    IRNode * node = newIRNode(RAW);
    node->comment = instruction;
    addNode(ir, node);
}

void addAdditionIR(IntermediateRepresentation *ir, int src1, int src2, int destination, int isFloat) {
    IRNode * node = newIRNode(ADD);
    node->dest = destination;
    node->sourceKind = REG_SOURCE;
    node->source = src1;
    node->source2 = src2;
    node->isFloat = isFloat;
    addNode(ir, node);
}

void addMultiplicationIR(IntermediateRepresentation *ir, int src1, int src2, int destination, int isFloat) {
    IRNode * node = newIRNode(MUL);
    node->dest = destination;
    node->sourceKind = REG_SOURCE;
    node->source = src1;
    node->source2 = src2;
    node->isFloat = isFloat;
    addNode(ir, node);
}

void addSubtractionIR(IntermediateRepresentation *ir, int src1, int src2, int destination, int isFloat) {
    IRNode * node = newIRNode(SUB);
    node->dest = destination;
    node->sourceKind = REG_SOURCE;
    node->source = src1;
    node->source2 = src2;
    node->isFloat = isFloat;
    addNode(ir, node);
}

void freeIntermediateRepresentation(IntermediateRepresentation *ir) {
    IRNode *node = ir->head;
    IRNode *next = NULL;
    while(node != NULL){
        next = node->next;
        free(node);
        node = next;
    }

    free(ir);
}

void addLabelIR(IntermediateRepresentation *ir, SymbolicTableEntry *entry) {
    IRNode * node = newIRNode(LABEL);
    node->varSource = entry;
    addNode(ir, node);
}

void addJumpIR(IntermediateRepresentation *ir, SymbolicTableEntry *entry) {
    IRNode * node = newIRNode(JUMP);
    node->sourceKind = VARIABLE_SOURCE;
    node->varSource = entry;
    addNode(ir, node);
}

void addRelativeJump(IntermediateRepresentation *ir, IRNode *target) {
    IRNode * node = newIRNode(RELATIVE_JUMP);
    node->target = target;
    addNode(ir, node);
}


void addGetPC(IntermediateRepresentation *ir, int destinationRegister, int imm) {
    IRNode * node = newIRNode(AUIPC);
    node->dest = destinationRegister;
    node->sourceKind = CONSTANT_SOURCE;
    node->imm = imm;
    addNode(ir, node);
}

void addGetPCVarAddress(IntermediateRepresentation *ir, int destinationRegister, SymbolicTableEntry *entry) {
    IRNode * node = newIRNode(AUIPC);
    node->dest = destinationRegister;
    node->sourceKind = VARIABLE_SOURCE;
    node->varSource = entry;
    addNode(ir, node);
}

void addLoadVarAddress(IntermediateRepresentation *ir, int destinationRegister, int sourceRegister, SymbolicTableEntry *entry) {
    IRNode * node = newIRNode(LOAD);
    node->sourceKind = VARIABLE_SOURCE;
    node->varSource = entry;
    node->dest = destinationRegister;
    node->source = sourceRegister;
    addNode(ir, node);
}

void addStoreVarAddress(IntermediateRepresentation *ir, int destinationRegister, int sourceRegister, SymbolicTableEntry *entry) {
    IRNode * node = newIRNode(STORE);
    node->sourceKind = VARIABLE_SOURCE;
    node->varSource = entry;
    node->dest = destinationRegister;
    node->source = sourceRegister;
    addNode(ir, node);
}

void addJumpRegisterIR(IntermediateRepresentation *ir, int destinationRegister) {
    IRNode * node = newIRNode(JUMP_REGISTER);
    node->dest = destinationRegister;
    addNode(ir, node);
}

void addNegIR(IntermediateRepresentation *ir, int dest, int src) {
    IRNode * node = newIRNode(NEG);
    node->dest = dest;
    node->source = src;
    addNode(ir, node);
}

void addAdditionImIR(IntermediateRepresentation *ir, int dest, int src, int imm) {
    IRNode * node = newIRNode(ADD);
    node->dest = dest;
    node->sourceKind = CONSTANT_SOURCE;
    node->source = src;
    node->imm = imm;
    addNode(ir, node);
}

IRNode *addJumpImIR(IntermediateRepresentation *ir, int imm) {
    IRNode * node = newIRNode(JUMP);
    node->sourceKind = CONSTANT_SOURCE;
    node->source = imm;
    addNode(ir, node);
    return node;
}

void addNopIR(IntermediateRepresentation *ir) {
    IRNode * node = newIRNode(NOP);
    addNode(ir, node); 
}

void addMovIR(IntermediateRepresentation *ir, int dest, int src) {
    IRNode * node = newIRNode(MOV);
    node->sourceKind = REG_SOURCE;
    node->source = src;
    node->dest = dest;
    addNode(ir, node);
}

void addFloatMovIR(IntermediateRepresentation *ir, int dest, int src) {
    IRNode * node = newIRNode(FMOV);
    node->sourceKind = REG_SOURCE;
    node->source = src;
    node->dest = dest;
    addNode(ir, node);
}

void addCSRReadWrite(IntermediateRepresentation *ir, int destination, int value, int source) {
    IRNode * node = newIRNode(CSRRW);
    node->sourceKind = REG_SOURCE;
    node->source = source;
    node->dest = destination;
    node->imm = value;
    addNode(ir, node);  
}

void addMovFromIntegerToFloat(IntermediateRepresentation *ir, int destination, int source) {
    IRNode * node = newIRNode(FMVWX);
    node->sourceKind = REG_SOURCE;
    node->source = source;
    node->dest = destination;
    addNode(ir, node);
}

IRNode* addBLTIR(IntermediateRepresentation *ir, int src1, int src2, int destination) {
    IRNode * node = newIRNode(BLT);
    node->imm = destination;
    node->sourceKind = REG_SOURCE;
    node->source = src1;
    node->source2 = src2;
    node->dest = X0_REGISTER;
    addNode(ir, node);
}

IRNode* addBGTIR(IntermediateRepresentation *ir, int src1, int src2, int destination) {
    IRNode * node = newIRNode(BGT);
    node->imm = destination;
    node->sourceKind = REG_SOURCE;
    node->source = src1;
    node->source2 = src2;
    node->dest = X0_REGISTER;
    addNode(ir, node);
}

IRNode* addBLEIR(IntermediateRepresentation *ir, int src1, int src2, int destination) {
    IRNode * node = newIRNode(BLE);
    node->imm = destination;
    node->sourceKind = REG_SOURCE;
    node->source = src1;
    node->source2 = src2;
    node->dest = X0_REGISTER;
    addNode(ir, node);
}

IRNode* addBGEIR(IntermediateRepresentation *ir, int src1, int src2, int destination) {
    IRNode * node = newIRNode(BGE);
    node->imm = destination;
    node->sourceKind = REG_SOURCE;
    node->source = src1;
    node->source2 = src2;
    node->dest = X0_REGISTER;
    addNode(ir, node);
}

IRNode* addBEQIR(IntermediateRepresentation *ir, int src1, int src2, int destination) {
    IRNode * node = newIRNode(BEQ);
    node->imm = destination;
    node->sourceKind = REG_SOURCE;
    node->source = src1;
    node->source2 = src2;
    node->dest = X0_REGISTER;
    addNode(ir, node);
}


IRNode* addBNEQIR(IntermediateRepresentation *ir, int src1, int src2, int destination) {
    IRNode * node = newIRNode(BNEQ);
    node->imm = destination;
    node->sourceKind = REG_SOURCE;
    node->source = src1;
    node->source2 = src2;
    node->dest = X0_REGISTER;
    addNode(ir, node);
    return node;
}
