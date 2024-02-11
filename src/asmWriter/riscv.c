#include "riscv.h"

typedef struct objectCode {
    char assembly[25];
    int binary; 
    struct objectCode *next;
} ObjectCode; 

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

void printIR(IntermediateRepresentation *ir, FILE *f_asm, FILE *f_bin, RegisterMapping *rm){
    ObjectCode *currObj = NULL, *objCode = (ObjectCode *) malloc(sizeof(ObjectCode));
    currObj = objCode;
    IRNode *p = ir->head;

    SymbolicTableGlobals *g = ir->globals->next;
    while (g!=NULL) {
        g->entry->address = ir->lastAddress;
        ir->lastAddress += getSize(g->entry->type);
        g = g->next;
    }

    while (p != NULL) {
        currObj->next = (ObjectCode *) malloc(sizeof(ObjectCode));
        currObj = currObj->next;
        currObj->next = NULL;
        strcpy(currObj->assembly, "");
        currObj->binary = 0x20;
        switch (p->instruction) {
            case MOV:
                sprintf(currObj->assembly, "mv %s", getReg(rm, p->dest));
                break;
            case ADD:
                sprintf(currObj->assembly, "add %s, %s, %s", 
                    getReg(rm, p->dest),
                    getReg(rm, p->source),
                    getReg(rm, p->source2)
                );
                break;
            case SUB:
                sprintf(currObj->assembly, "sub %s, %s, %s", 
                    getReg(rm, p->dest),
                    getReg(rm, p->source),
                    getReg(rm, p->source2)
                );
                break;
            case LOAD:
                if (p->sourceKind == CONSTANT_SOURCE)
                    sprintf(currObj->assembly, "li %s, %d", 
                        getReg(rm, p->dest), 
                        p->source
                    );
                else if  (p->sourceKind == VARIABLE_SOURCE) {
                    sprintf(currObj->assembly, "li %s, %d", 
                        getReg(rm, p->dest), 
                        p->varSource->address
                    );
                } else {
                    sprintf(currObj->assembly, "lw %s, 0(%s)", 
                        getReg(rm, p->dest), 
                        getReg(rm, p->source)
                    );
                }
                break;
            case STORE:
                sprintf(currObj->assembly, "sw %s, 0(%s)", getReg(rm, p->source), getReg(rm, p->dest));
                break;
            case COMMENT:
                sprintf(currObj->assembly, "//%s", p->comment);
                break;
            case JUMP:
                sprintf(currObj->assembly, "j %d", p->varSource->address);
                break;
            case LABEL:
                sprintf(currObj->assembly, "%s: nop", p->comment);
                break;

        }
        p = p->next;
    }

    currObj = objCode->next;
    while (currObj != NULL){
        fprintf(f_asm, "%s\n", currObj->assembly);
        fwrite(&currObj->binary, 4, 1, f_bin);
        currObj = currObj->next;
    }

    currObj = objCode;
    while (currObj != NULL){
        objCode = currObj;
        currObj = currObj->next;
        free(objCode);
    }
}