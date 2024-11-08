#include "riscv.h"
#include "binary.h"

int registerCodes[13] = {
    5, 6, 7, 28, 29, 30, 31, 12, 13, 14, 15, 16, 17
};

char *getRegisterDialect1(RegisterAssignment *rm, int temporary){
    char *registerNames[13] = {
        "x5", "x6", "x7", "x28", "x29", "x30", "x31", "x12", "x13", "x14", "x15", "x16", "x17"
    };
    return registerNames[getRegisterAssignment(rm, temporary)];
}

char *getRegisterDialect2(RegisterAssignment *rm, int temporary){
    if (rm == NULL) {
        char *str = malloc(5); 
        sprintf(str, "#%d", temporary);
        return str;
    }
    char *registerNames[13] = {
        "t0", "t1", "t2", "t3", "t4", "t5", "t6", "a2", "a3", "a4", "a5", "a6", "a7"
    };
    char *registerIndex[32] = {
        "zero", "ra", "sp",  "gp",  "tp", "t0",  "t1",  "t2",
        "s0",   "s1", "a0",  "a1",  "a2", "a3",  "a4",  "a5",
        "a6",   "a7", "s2",  "s3",  "s4", "s5",  "s6",  "s7",
        "s8",   "s9", "s10", "s11", "t3", "t4",  "t5",  "t6"
    };
    // way to identify fixed temporaries - avoid allocation
    if (temporary < 0)
    return registerIndex[-1-temporary];
    int regIndex = getRegisterAssignment(rm, temporary);
    if (regIndex < 0 || regIndex > 12) {
        char *str = malloc(5); 
        sprintf(str, "#%d", temporary);
        return str;
    }
    return registerNames[regIndex];
}

char *getFloatRegisterDialect2(RegisterAssignment *rm, int temporary){
    if (rm == NULL) {
        char *str = malloc(5); 
        sprintf(str, "#%d", temporary);
        return str;
    }
    // used by allocation
    char *registerNames[13] = {
        "ft0", "ft1", "ft2", "ft3", "ft4", "ft5", "ft6", "ft7", 
        "ft8", "ft9", "ft10", "ft11", "fa1"
    };
    char *registerIndex[32] = {
        "ft0", "ft1", "ft2",  "ft3",  "ft4", "ft5", "ft6",  "ft7",
        "fs0", "fs1", "fa0",  "fa1",  "fa2", "fa3", "fa4",  "fa5",
        "fa6", "fa7", "fs2",  "fs3",  "fs4", "fs5", "fs6",  "fs7",
        "fs8", "fs9", "fs10", "fs11", "ft8", "ft9", "ft10", "ft11"
    };
    // way to identify fixed temporaries - avoid allocation
    if (temporary < 0)
        return registerIndex[-1-temporary];

    int regIndex = getRegisterAssignment(rm, temporary);
    if (regIndex < 0 || regIndex > 12) {
        char *str = malloc(5); 
        sprintf(str, "#%d", temporary);
        return str;
    }
    return registerNames[regIndex];
}

typedef char* (*getRegisterFunction)(RegisterAssignment*, int);

getRegisterFunction getRegisterGenerator(int dialect) {
    if (dialect == 1) {
        return getRegisterDialect1;
    } else if (dialect == 2) {
        return getRegisterDialect2;
    }
}

int getRegBin(RegisterAssignment *rm, int temporary){
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

ObjectCode *translateIRToObj(IntermediateRepresentation *ir, RegisterAssignment *rm, int includeASMComments, int asmDialect){
    ObjectCode *currObj = NULL, *objCode = NULL;
    currObj = objCode;
    IRNode *p = ir->head;
    getRegisterFunction getReg = getRegisterGenerator(asmDialect);
    getRegisterFunction getFloatReg = getFloatRegisterDialect2;
    int i = 0;

    while (p != NULL) {
        // printf()
        if (objCode != NULL) {
            currObj->next = newObjectCode();
            currObj = currObj->next;
        } else {
            objCode = newObjectCode();
            currObj = objCode;
        }

        switch (p->instruction) {
            case ADD:
                if (p->isFloat)
                    sprintf(currObj->assembly, "fadd.s %s, %s, %s", 
                        getFloatReg(rm, p->dest),
                        getFloatReg(rm, p->source),
                        getFloatReg(rm, p->source2)
                    );
                else {
                    if (p->sourceKind == CONSTANT_SOURCE){
                        sprintf(currObj->assembly, "addi %s, %s, %d", 
                            getReg(rm, p->dest),
                            getReg(rm, p->source),
                            p->imm
                        );
                    }
                    else if (p->sourceKind == REG_SOURCE)
                        sprintf(currObj->assembly, "add %s, %s, %s", 
                            getReg(rm, p->dest),
                            getReg(rm, p->source),
                            getReg(rm, p->source2)
                        );
                    else sprintf(currObj->assembly, "addi %s, %s, %d", 
                            getReg(rm, p->dest),
                            getReg(rm, p->dest),
                            ((p->varSource->address - p->address) & ((1 << 12) - 1)) + p->imm
                        );
                }
                
                break;
            case MUL:
                if (p->isFloat)
                    sprintf(currObj->assembly, "fmul.s %s, %s, %s", 
                        getFloatReg(rm, p->dest),
                        getFloatReg(rm, p->source),
                        getFloatReg(rm, p->source2)
                    );
                else
                sprintf(currObj->assembly, "mul %s, %s, %s", 
                    getReg(rm, p->dest),
                    getReg(rm, p->source),
                    getReg(rm, p->source2)
                );
                
                break;
            case SUB:
                if (p->isFloat)
                    sprintf(currObj->assembly, "fsub.s %s, %s, %s", 
                        getFloatReg(rm, p->dest),
                        getFloatReg(rm, p->source),
                        getFloatReg(rm, p->source2)
                    );
                else sprintf(currObj->assembly, "sub %s, %s, %s", 
                    getReg(rm, p->dest),
                    getReg(rm, p->source),
                    getReg(rm, p->source2)
                );
                break;
            case FLT:
                sprintf(currObj->assembly, "flt.s %s, %s, %s", 
                    getReg(rm, p->dest),
                    getFloatReg(rm, p->source),
                    getFloatReg(rm, p->source2)
                );
                break;
            case MACC:
                sprintf(currObj->assembly, "macc");
                break;
            case MACCSTORE:
                sprintf(currObj->assembly, "store %d", p->imm);
                break;
            case SETPA:
                sprintf(currObj->assembly, "setpa %d", p->imm);
                break;
            case SETPB:
                sprintf(currObj->assembly, "setpb %d", p->imm);
                break;
            case FLE:
                sprintf(currObj->assembly, "fle.s %s, %s, %s", 
                    getReg(rm, p->dest),
                    getFloatReg(rm, p->source),
                    getFloatReg(rm, p->source2)
                );
                break;
            case FSGNJ:
                sprintf(currObj->assembly, "fsgnj.s %s, %s, %s", 
                    getFloatReg(rm, p->dest),
                    getFloatReg(rm, p->source),
                    getFloatReg(rm, p->source2)
                );
                break;
            case FSGNJN:
                sprintf(currObj->assembly, "fsgnjn.s %s, %s, %s", 
                    getFloatReg(rm, p->dest),
                    getFloatReg(rm, p->source),
                    getFloatReg(rm, p->source2)
                );
                break;
            case NEG:
                sprintf(currObj->assembly, "neg %s, %s", 
                    getReg(rm, p->dest),
                    getReg(rm, p->source)
                );
                break;
            case PREPARE_STACK:
                sprintf(currObj->assembly, "auipc %s, %d", 
                        getReg(rm, SP_REGISTER),
                        ((ir->tail->address - ir->head->address) >> 12) + 3
                    );
                break;
            case AUIPC:
                if (p->sourceKind == CONSTANT_SOURCE)
                    sprintf(currObj->assembly, "auipc %s, %d", 
                        getReg(rm, p->dest),
                        p->imm
                    );
                else if  (p->sourceKind == VARIABLE_SOURCE) {
                    sprintf(currObj->assembly, "auipc %s, %d", 
                        getReg(rm, p->dest),
                        ((p->varSource->address-p->address) + 0x800)  >> 12
                    );
                }
                break;      
            case CSRRW:
                sprintf(currObj->assembly, "csrrw %s, %d, %s", 
                    getReg(rm, p->dest),
                    p->imm,
                    getReg(rm, p->source)
                );
                break;      
            case LOAD:
                if (p->isFloat && p->sourceKind == VARIABLE_SOURCE) {
                    printf("Adding LOAD for distance %d which is %d and %d\n", p->varSource->address-p->address,  ((p->varSource->address-p->address) + 0x800)  >> 12, (p->varSource->address-p->address) & ((1 << 12) - 1));

                    sprintf(currObj->assembly, "flw %s, %d(%s)", 
                        getFloatReg(rm, p->dest), 
                        (p->varSource->address-p->address) & ((1 << 12) - 1),
                        getReg(rm, p->source)
                    );
                } else if (p->isFloat && p->sourceKind == REG_SOURCE) {
                    sprintf(currObj->assembly, "flw %s, %d(%s)", 
                        getFloatReg(rm, p->dest), 
                        p->imm,
                        getReg(rm, p->source)
                    );
                }
                else if (p->sourceKind == CONSTANT_SOURCE)
                    sprintf(currObj->assembly, "addi %s, %s, %d", 
                        getReg(rm, p->dest), 
                        getReg(rm, X0_REGISTER), 
                        p->source
                    );
                else if  (p->sourceKind == VARIABLE_SOURCE) {
                    sprintf(currObj->assembly, "lw %s, %d(%s)", 
                        getReg(rm, p->dest), 
                        (p->varSource->address-p->address) & ((1 << 12) - 1),
                        getReg(rm, p->source)
                    );
                } else {
                    sprintf(currObj->assembly, "lw %s, %d(%s)", 
                        getReg(rm, p->dest), 
                        p->imm,
                        getReg(rm, p->source)
                    );
                }
                break;  
            case LUI:
                sprintf(currObj->assembly, "lui %s, %d", 
                    getReg(rm, p->dest), 
                    p->source
                );
                break;
            case STORE:
                if (p->isFloat && p->sourceKind == VARIABLE_SOURCE) {
                    sprintf(currObj->assembly, "fsw %s, %d(%s)", 
                        getFloatReg(rm, p->dest), 
                        (p->varSource->address-p->address) & ((1 << 12) - 1),
                        getReg(rm, p->source)
                    );
                }
                else if  (p->sourceKind == VARIABLE_SOURCE) {
                    sprintf(currObj->assembly, "sw %s, %d(%s)", 
                        getReg(rm, p->dest), 
                        (p->varSource->address-p->address) & ((1 << 12) - 1),
                        getReg(rm, p->source)
                    );
                }
                else if (p->isFloat)
                    sprintf(currObj->assembly, "fsw %s, %d(%s)", getFloatReg(rm, p->source), p->imm, getReg(rm, p->dest));
                else 
                    sprintf(currObj->assembly, "sw %s, %d(%s)", getReg(rm, p->source), p->imm, getReg(rm, p->dest));
                break;
            case RAW:
                sprintf(currObj->assembly, "%s", p->comment);
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
                sprintf(currObj->assembly, "\n%s:\naddi %s, %s, 0", p->varSource->name, getReg(rm, X0_REGISTER), getReg(rm, X0_REGISTER));
                break;
            case JUMP_REGISTER:
                sprintf(currObj->assembly, "jalr %s, 0(%s)", getReg(rm, RA_REGISTER), getReg(rm, p->dest));
                break;
            case MOV:
                sprintf(currObj->assembly, "mv %s, %s", getReg(rm, p->dest), getReg(rm, p->source));
                break;
            case FMOV:
                sprintf(currObj->assembly, "fsgnj.s %s, %s, %s", getFloatReg(rm, p->dest), getFloatReg(rm, p->source), getFloatReg(rm, p->source));
                break;
            case FMVWX:
                sprintf(currObj->assembly, "fmv.w.x %s, %s", getFloatReg(rm, p->dest), getReg(rm, p->source));
                break;
            case BEQ:
                sprintf(currObj->assembly, "beq %s, %s, %d", getReg(rm, p->source), getReg(rm, p->source2), p->imm);
                break;
            case BNEQ:
                sprintf(currObj->assembly, "bne %s, %s, %d", getReg(rm, p->source), getReg(rm, p->source2), p->imm);
                break;
            case BLE:
                sprintf(currObj->assembly, "bge %s, %s, %d", getReg(rm, p->source2),  getReg(rm, p->source), p->imm);
                break;
            case BGE:
                sprintf(currObj->assembly, "bge %s, %s, %d", getReg(rm, p->source), getReg(rm, p->source2), p->imm);
                break;
            case BLT:
                sprintf(currObj->assembly, "blt %s, %s, %d", getReg(rm, p->source), getReg(rm, p->source2), p->imm);
                break;
            case BGT:
                sprintf(currObj->assembly, "blt %s, %s, %d", getReg(rm, p->source2), getReg(rm, p->source), p->imm);
                break;
            case NOP:
                sprintf(currObj->assembly, "addi %s, %s, 0", getReg(rm, X0_REGISTER), getReg(rm, X0_REGISTER));
                break;
            case RELATIVE_JUMP:
                sprintf(currObj->assembly, "jal %s, %d", getReg(rm, RA_REGISTER), p->target->address-p->address);
                break;
        }
        if (p->instruction == LABEL || p->instruction == COMMENT)
            currObj->binary = asmToBinary("NOP");
        else if (p->instruction == DATA)
            currObj->binary = p->imm;
        else currObj->binary = asmToBinary(currObj->assembly);
        // printf("%s was translated to %d\n", currObj->assembly, currObj->binary);
        
        // printf("%d: %s \n", i, currObj->assembly);
        i = i+1;
        p = p->next;
    }

    return objCode;
}

