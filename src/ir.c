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
    else ir->tail->next = node;
}

void addMovIR(IntermediateRepresentation *ir, int destination) {
    IRNode * node = (IRNode *) malloc(sizeof(IRNode));
    node->instruction = MOV;
    node->dest = destination;
    addNode(ir, node);
}

void printIR(IntermediateRepresentation *ir, FILE *f){
    IRNode *p = ir->head;
    while (p != NULL){
        switch (p->instruction) {
            case MOV:
                fprintf(f, "mv\n");
                break;
            case LOAD:
                fprintf(f, "li\n");
                break;
            case STORE:
                fprintf(f, "st\n");
                break;
        }
        p = p->next;
    }
}