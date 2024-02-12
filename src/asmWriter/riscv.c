#include "riscv.h"

typedef struct objectCode {
    char assembly[25];
    int binary; 
    struct objectCode *next;
    int include;
} ObjectCode; 

char *registerNames[7] = {
    "t0", "t1", "t2", "t3", "t4", "t5", "t6"
};

int registerCodes[7] = {
    5, 6, 7, 28, 29, 30, 31
};

char *getReg(RegisterMapping *rm, int temporary){
    if (temporary == SP_REGISTER)
        return "sp"; //sp or x2
    else if (temporary == A0_REGISTER)
        return "a0"; //a0 or x10
    else if (temporary == X0_REGISTER)
        return "zero"; 
    else if (temporary == RA_REGISTER)
        return "ra"; 
    return registerNames[getRegisterAssignment(rm, temporary)];
}

int getRegBin(RegisterMapping *rm, int temporary){
    if (temporary == SP_REGISTER)
        return 2; //sp or x2
    else if (temporary == A0_REGISTER)
        return 10; //a0 or x10
    else if (temporary == X0_REGISTER)
        return 0; 
    else if (temporary == RA_REGISTER)
        return 1; 
    return registerCodes[getRegisterAssignment(rm, temporary)];
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
                if (p->sourceKind == CONSTANT_SOURCE){
                    sprintf(currObj->assembly, "addi %s, %s, %d", 
                        getReg(rm, p->dest),
                        getReg(rm, p->source),
                        p->source2
                    );
                    currObj->binary = 0b0 << 20 
                                | getRegBin(rm, p->source) << 15 
                                | 0b000 << 12 
                                | getRegBin(rm, p->dest) << 7
                                | 0b0010011;
                }
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
                    sprintf(currObj->assembly, "jal %s, %d", getReg(rm, RA_REGISTER), p->source);
                else sprintf(currObj->assembly, "jal %s, %d", 
                    getReg(rm, RA_REGISTER), 
                    p->varSource->address - p->address
                );
                break;
            case LABEL:
                sprintf(currObj->assembly, "\n%s:\naddi zero, zero, 0", p->varSource->name);
                currObj->binary = 0b0 << 20 | 0b00000 << 15 | 0b000 << 12 | 0b00000 << 7 | 0b0010011;
                break;
            case JUMP_REGISTER:
                sprintf(currObj->assembly, "jalr %s, 0(%s)", getReg(rm, RA_REGISTER), getReg(rm, p->dest));
                break;
            case MOV:
                sprintf(currObj->assembly, "mv %s, %s", getReg(rm, p->dest), getReg(rm, p->source));
                break;
            case NOP:
                sprintf(currObj->assembly, "addi zero, zero, 0");
                currObj->binary = 0b0 << 20 
                                | 0 << 15 
                                | 0b000 << 12 
                                | 0 << 7
                                | 0b0010011;
                break;
        }
        p = p->next;
    }

    currObj = objCode->next;
    while (currObj != NULL){
        if (currObj->include) {
            fprintf(f_asm, "%s\n", currObj->assembly);
            fwrite(&currObj->binary, sizeof(currObj->binary), 1, f_bin);
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