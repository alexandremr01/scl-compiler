#include "ir.h"

IntermediateRepresentation *newIntermediateRepresentation(){
    IntermediateRepresentation *ir = (IntermediateRepresentation *) malloc(sizeof(IntermediateRepresentation));
    ir->head = NULL;
    ir->tail = NULL;
    ir->nextTempReg = 0;
    ir->lastAddress = 0;
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
    if (node->instruction != COMMENT) 
        ir->lastAddress += 4;
}

IRNode *newIRNode(Instruction instruction){
    IRNode * node = (IRNode *) malloc(sizeof(IRNode));
    node->instruction = instruction;
    node->next = NULL;
    return node;
}

void addMovIR(IntermediateRepresentation *ir, int destination) {
    IRNode * node = newIRNode(MOV);
    node->dest = destination;
    addNode(ir, node);
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
    addNode(ir, node);
    // TODO: Store shift
}

void addLoadMemIR(IntermediateRepresentation *ir, int destination_address, int shift, int register_source){
    IRNode * node = newIRNode(LOAD);
    node->dest = destination_address;
    node->sourceKind = REG_SOURCE;
    node->source = register_source;
    addNode(ir, node);
    // TODO: Store shift
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

void addLabelIR(IntermediateRepresentation *ir, char *name) {
    IRNode * node = newIRNode(LABEL);
    node->comment = name;
    addNode(ir, node);
}

void addJumpIR(IntermediateRepresentation *ir, SymbolicTableEntry *entry) {
    IRNode * node = newIRNode(JUMP);
    node->varSource = entry;
    addNode(ir, node);
}