#include "riscv.h"

char *registerNames[6] = {
    "rx0", "rx1", "rx2", "rx3", "rx4", "rx5"
};

char *getReg(RegisterMapping *rm, int temporary){
    return registerNames[getRegisterAssignment(rm, temporary)];
}

int getSize(DataType d){
    switch(d){
        case NONE_TYPE:
            return 0;
        case INTEGER_TYPE:
            return 4;
        case VOID_TYPE:
            return 0;
        case FLOAT_TYPE:
            return 4;
    }
    return 0;
}

void printIR(IntermediateRepresentation *ir, FILE *f, RegisterMapping *rm){
    IRNode *p = ir->head;

    SymbolicTableGlobals *g = ir->globals->next;
    while (g!=NULL) {
        g->entry->address = ir->lastAddress;
        ir->lastAddress += getSize(g->entry->type);
        g = g->next;
    }

    while (p != NULL) {
        switch (p->instruction) {
            case MOV:
                fprintf(f, "mv %s\n", getReg(rm, p->dest));
                break;
            case ADD:
                fprintf(f, "add %s, %s, %s\n", 
                    getReg(rm, p->dest),
                    getReg(rm, p->source),
                    getReg(rm, p->source2)
                );
                break;
            case SUB:
                fprintf(f, "sub %s, %s, %s\n", 
                    getReg(rm, p->dest),
                    getReg(rm, p->source),
                    getReg(rm, p->source2)
                );
                break;
            case LOAD:
                if (p->sourceKind == CONSTANT_SOURCE)
                    fprintf(f, "li %s, %d\n", 
                        getReg(rm, p->dest), 
                        p->source
                    );
                else if  (p->sourceKind == VARIABLE_SOURCE) {
                    fprintf(f, "li %s, %d\n", 
                        getReg(rm, p->dest), 
                        p->varSource->address
                    );
                } else {
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