#include "ir.h"

IntermediateRepresentation *newIntermediateRepresentation(){
    IntermediateRepresentation *ir = (IntermediateRepresentation *) malloc(sizeof(IntermediateRepresentation));
    ir->head = NULL;
    ir->tail = NULL;
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

void printIR(IntermediateRepresentation *ir, FILE *f){
    IRNode *p = ir->head;
    while (p != NULL){
        switch (p->instruction) {
            case MOV:
                fprintf(f, "mv #%d\n", p->dest);
                break;
            case LOAD:
                if (p->sourceKind == CONSTANT_SOURCE)
                    fprintf(f, "li #%d, %d\n", p->dest, p->source);
                else fprintf(f, "lw #%d, 0(#%d)\n", p->dest, p->source);
                break;
            case STORE:
                fprintf(f, "sw #%d, 0(#%d)\n", p->source, p->dest);
                break;
            case COMMENT:
                fprintf(f, "//%s\n", p->comment);
                break;
        }
        p = p->next;
    }
}