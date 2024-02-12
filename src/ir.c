#include "ir.h"

IntermediateRepresentation *newIntermediateRepresentation(){
    IntermediateRepresentation *ir = (IntermediateRepresentation *) malloc(sizeof(IntermediateRepresentation));
    ir->head = NULL;
    ir->tail = NULL;
    ir->nextTempReg = 0;
    ir->lastAddress = 0;
    ir->lastStackAddress = 0;
    return ir;
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
    return node;
}

void addLoadImIR(IntermediateRepresentation *ir, int destination, int value) {
    IRNode * node = newIRNode(LOAD);
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
    node->source2 = shift;
    addNode(ir, node);
}

void addLoadMemIR(IntermediateRepresentation *ir, int destination_address, int shift, int register_source){
    IRNode * node = newIRNode(LOAD);
    node->dest = destination_address;
    node->sourceKind = REG_SOURCE;
    node->source = register_source;
    node->source2 = shift;
    addNode(ir, node);
}

void addCommentIR(IntermediateRepresentation *ir, char *comment){
    IRNode * node = newIRNode(COMMENT);
    node->comment = comment;
    addNode(ir, node);
}

void addAdditionIR(IntermediateRepresentation *ir, int src1, int src2, int destination) {
    IRNode * node = newIRNode(ADD);
    node->dest = destination;
    node->sourceKind = REG_SOURCE;
    node->source = src1;
    node->source2 = src2;
    addNode(ir, node);
}

void addSubtractionIR(IntermediateRepresentation *ir, int src1, int src2, int destination) {
    IRNode * node = newIRNode(SUB);
    node->dest = destination;
    node->sourceKind = REG_SOURCE;
    node->source = src1;
    node->source2 = src2;
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

    SymbolicTableGlobals *g = ir->globals;
    SymbolicTableGlobals *nextGlobal = NULL;
    while(g != NULL){
        nextGlobal = g->next;
        free(g);
        g = nextGlobal;
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

void addJumpRegisterIR(IntermediateRepresentation *ir, int destinationRegister) {
    IRNode * node = newIRNode(JUMP_REGISTER);
    node->dest = destinationRegister;
    addNode(ir, node);
}

void addAdditionImIR(IntermediateRepresentation *ir, int dest, int src, int imm) {
    IRNode * node = newIRNode(ADD);
    node->dest = dest;
    node->sourceKind = CONSTANT_SOURCE;
    node->source = src;
    node->source2 = imm;
    addNode(ir, node);
}

void addJumpImIR(IntermediateRepresentation *ir, int imm) {
    IRNode * node = newIRNode(JUMP);
    node->sourceKind = CONSTANT_SOURCE;
    node->source = imm;
    addNode(ir, node);
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
