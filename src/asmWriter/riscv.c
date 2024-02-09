#include "riscv.h"

void printIR(IntermediateRepresentation *ir, FILE *f, RegisterMapping *rm){
    IRNode *p = ir->head;
    while (p != NULL){
        switch (p->instruction) {
            case MOV:
                fprintf(f, "mv %s\n", getRegister(rm, p->dest));
                break;
            case LOAD:
                if (p->sourceKind == CONSTANT_SOURCE)
                    fprintf(f, "li %s, %d\n", 
                        getRegister(rm, p->dest), 
                        p->source
                    );
                else {
                    fprintf(f, "lw %s, 0(%s)\n", 
                        getRegister(rm, p->dest), 
                        getRegister(rm, p->source)
                    );
                }
                break;
            case STORE:
                fprintf(f, "sw %s, 0(%s)\n", getRegister(rm, p->source), getRegister(rm, p->dest));
                break;
            case COMMENT:
                fprintf(f, "//%s\n", p->comment);
                break;
        }
        p = p->next;
    }
}