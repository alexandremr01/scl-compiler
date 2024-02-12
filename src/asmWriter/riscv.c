#include "riscv.h"

typedef struct objectCode {
    char assembly[25];
    int binary; 
    struct objectCode *next;
    int include;
} ObjectCode; 

char *registerNames[7] = {
    "x5", "x6", "x7", "x28", "x29", "x30", "x31"
};

char *getReg(RegisterMapping *rm, int temporary){
    if (temporary == SP_REGISTER)
        return "sp"; //sp or x2
    else if (temporary == A0_REGISTER)
        return "a0"; //a0 or x10
    else if (temporary == X0_REGISTER)
        return "x0"; 
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

void printIR(IntermediateRepresentation *ir, FILE *f_asm, FILE *f_bin, RegisterMapping *rm, int includeASMComments){
    ObjectCode *currObj = NULL, *objCode = (ObjectCode *) malloc(sizeof(ObjectCode));
    currObj = objCode;
    IRNode *p = ir->head;

    // give address to globals
    SymbolicTableGlobals *g = ir->globals->next;
    while (g!=NULL) {
        g->entry->address = ir->lastAddress;
        ir->lastAddress += getSize(g->entry->type);
        g = g->next;
    }

    // give label address to variables
    while (p != NULL) {
        if (p->instruction == LABEL){
            p->varSource->address = p->address;
        }
        p = p->next;
    }

    p = ir->head;
    while (p != NULL) {
        currObj->next = (ObjectCode *) malloc(sizeof(ObjectCode));
        currObj = currObj->next;
        currObj->next = NULL;
        currObj->include = 1;
        strcpy(currObj->assembly, "");
        currObj->binary = 0x20;
        switch (p->instruction) {
            case ADD:
                if (p->sourceKind == CONSTANT_SOURCE)
                    sprintf(currObj->assembly, "addi %s, %s, %d", 
                        getReg(rm, p->dest),
                        getReg(rm, p->source),
                        p->source2
                    );
                else sprintf(currObj->assembly, "add %s, %s, %s", 
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
                    sprintf(currObj->assembly, "lw %s, %d(%s)", 
                        getReg(rm, p->dest), 
                        p->source2,
                        getReg(rm, p->source)
                    );
                }
                break;
            case STORE:
                sprintf(currObj->assembly, "sw %s, %d(%s)", getReg(rm, p->source), p->source2, getReg(rm, p->dest));
                break;
            case COMMENT:
                sprintf(currObj->assembly, "// %s", p->comment);
                currObj->include = includeASMComments;
                break;
            case JUMP:
                if (p->sourceKind == CONSTANT_SOURCE)
                    sprintf(currObj->assembly, "jal x0, %d", p->source);
                else sprintf(currObj->assembly, "jal x0, %d", p->varSource->address);
                break;
            case LABEL:
                sprintf(currObj->assembly, "\n%s: addi rx0, rx0, 0", p->varSource->name);
                break;
            case JUMP_REGISTER:
                sprintf(currObj->assembly, "jalr x0, 0(%s)", getReg(rm, p->dest));
                break;
            case MOV:
                sprintf(currObj->assembly, "mv %s, %s", getReg(rm, p->dest), getReg(rm, p->source));
                break;
            case NOP:
                sprintf(currObj->assembly, "addi rx0, rx0, 0");
                break;
        }
        p = p->next;
    }

    currObj = objCode->next;
    while (currObj != NULL){
        if (currObj->include) {
            fprintf(f_asm, "%s\n", currObj->assembly);
            fwrite(&currObj->binary, 4, 1, f_bin);
        }
        currObj = currObj->next;
    }

    currObj = objCode;
    while (currObj != NULL){
        objCode = currObj;
        currObj = currObj->next;
        free(objCode);
    }
}