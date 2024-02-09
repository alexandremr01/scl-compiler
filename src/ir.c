#include "ir.h"

IntermediateRepresentation *newIntermediateRepresentation(){
    IntermediateRepresentation *ir = (IntermediateRepresentation *) malloc(sizeof(IntermediateRepresentation));
    ir->head = NULL;
    ir->tail = NULL;
    ir->nextTempReg = 0;
    return ir;
}

IRNode *newIRNode() {
    return (IRNode *) malloc(sizeof(IRNode));
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

void addMovIR(IntermediateRepresentation *ir, int destination) {
    IRNode * node = (IRNode *) malloc(sizeof(IRNode));
    node->instruction = MOV;
    node->dest = destination;
    addNode(ir, node);
}

void addLoadImIR(IntermediateRepresentation *ir, int destination, int value) {
    IRNode * node = (IRNode *) malloc(sizeof(IRNode));
    node->instruction = LOAD;
    node->dest = destination;
    node->sourceKind = CONSTANT_SOURCE;
    node->source = value;
    addNode(ir, node);
}

void addStoreIR(IntermediateRepresentation *ir, int destination_address, int shift, int register_source){
    IRNode * node = (IRNode *) malloc(sizeof(IRNode));
    node->instruction = STORE;
    node->dest = destination_address;
    node->sourceKind = REG_SOURCE;
    node->source = register_source;
    addNode(ir, node);
    // TODO: Store shift
}

void addLoadMemIR(IntermediateRepresentation *ir, int destination_address, int shift, int register_source){
    IRNode * node = (IRNode *) malloc(sizeof(IRNode));
    node->instruction = LOAD;
    node->dest = destination_address;
    node->sourceKind = REG_SOURCE;
    node->source = register_source;
    addNode(ir, node);
    // TODO: Store shift
}

void addCommentIR(IntermediateRepresentation *ir, char *comment){
    IRNode * node = (IRNode *) malloc(sizeof(IRNode));
    node->instruction = COMMENT;
    node->comment = comment;
    addNode(ir, node);
}

