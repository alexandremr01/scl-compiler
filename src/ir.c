#include "ir.h"

IntermediateRepresentation *newIntermediateRepresentation(){
    IntermediateRepresentation *ir = (IntermediateRepresentation *) malloc(sizeof(IntermediateRepresentation));
    ir->head = NULL;
    ir->tail = NULL;
    ir->nextTempReg = 0;
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

