#include "riscv.h"

char *registerNames[6] = {
    "rx0", "rx1", "rx2", "rx3", "rx4", "rx5"
};

char *getReg(RegisterMapping *rm, int temporary){
    return registerNames[getRegisterAssignment(rm, temporary)];
}

void printIR(IntermediateRepresentation *ir, FILE *f, RegisterMapping *rm){
    IRNode *p = ir->head;

    while (p != NULL) {
        switch (p->instruction) {
            case MOV:
                fprintf(f, "mv %s\n", getReg(rm, p->dest));
                break;
            case LOAD:
                if (p->sourceKind == CONSTANT_SOURCE)
                    fprintf(f, "li %s, %d\n", 
                        getReg(rm, p->dest), 
                        p->source
                    );
                else {
                    fprintf(f, "lw %s, 0(%s)\n", 
                        getReg(rm, p->dest), 
                        getReg(rm, p->source)
                    );
                }
                break;
            case STORE:
                fprintf(f, "sw %s, 0(%s)\n", getReg(rm, p->source), getReg(rm, p->dest));
                break;
            case COMMENT:
                fprintf(f, "//%s\n", p->comment);
                break;
        }
        p = p->next;
    }
}