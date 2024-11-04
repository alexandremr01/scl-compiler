#include "riscv.h"
#include "binary.h"

int registerCodes[13] = {
    5, 6, 7, 28, 29, 30, 31, 12, 13, 14, 15, 16, 17
};

char *getRegisterDialect1(RegisterAssignment *rm, int temporary){
    char *registerNames[13] = {
        "x5", "x6", "x7", "x28", "x29", "x30", "x31", "x12", "x13", "x14", "x15", "x16", "x17"
    };
    if (temporary == SP_REGISTER)
        return "x2"; //sp or x2
    else if (temporary == A0_REGISTER)
        return "x10"; //a0 or x10
    else if (temporary == X0_REGISTER)
        return "x0"; 
    else if (temporary == RA_REGISTER)
        return "x1"; 
    else if (temporary >= T0_REGISTER && temporary <= T6_REGISTER)
        return registerNames[temporary - T0_REGISTER]; 
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
    if (temporary == SP_REGISTER)
        return "sp";
    else if (temporary == A0_REGISTER)
        return "a0";
    else if (temporary == X0_REGISTER)
        return "zero"; 
    else if (temporary == RA_REGISTER)
        return "ra"; 
    else if (temporary >= T0_REGISTER && temporary <= T6_REGISTER)
        return registerNames[temporary - T0_REGISTER]; 
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
    char *registerNames[12] = {
        "f0", "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8",
        "f9", "f10", "f11"
    };

    if (temporary == FA0_REGISTER)
        return "f10";
    else if (temporary == SP_REGISTER)
        return "sp";
    else if (temporary == A0_REGISTER)
        return "a0";
    else if (temporary == X0_REGISTER)
        return "zero"; 
    else if (temporary == RA_REGISTER)
        return "ra"; 
    else if (temporary >= T0_REGISTER && temporary <= T6_REGISTER)
        return registerNames[temporary - T0_REGISTER]; 
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
                    else sprintf(currObj->assembly, "add %s, %s, %s", 
                        getReg(rm, p->dest),
                        getReg(rm, p->source),
                        getReg(rm, p->source2)
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
            case NEG:
                sprintf(currObj->assembly, "neg %s, %s", 
                    getReg(rm, p->dest),
                    getReg(rm, p->source)
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
                if (p->sourceKind == CONSTANT_SOURCE)
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
                if  (p->sourceKind == VARIABLE_SOURCE) {
                    sprintf(currObj->assembly, "sw %s, %d(%s)", 
                        getReg(rm, p->dest), 
                        (p->varSource->address-p->address) & ((1 << 12) - 1),
                        getReg(rm, p->source)
                    );
                }
                else sprintf(currObj->assembly, "sw %s, %d(%s)", getReg(rm, p->source), p->imm, getReg(rm, p->dest));
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

