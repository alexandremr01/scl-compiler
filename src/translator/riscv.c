#include "riscv.h"
#include "binary.h"

char *registerNames[13] = {
    "t0", "t1", "t2", "t3", "t4", "t5", "t6", "a2", "a3", "a4", "a5", "a6", "a7"
};

int registerCodes[13] = {
    5, 6, 7, 28, 29, 30, 31, 12, 13, 14, 15, 16, 17
};

char *getReg(RegisterAssignment *rm, int temporary){
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

// char *getReg(RegisterAssignment *rm, int temporary){
//     if (temporary == SP_REGISTER)
//         return "sp"; //sp or x2
//     else if (temporary == A0_REGISTER)
//         return "a0"; //a0 or x10
//     else if (temporary == X0_REGISTER)
//         return "zero"; 
//     else if (temporary == RA_REGISTER)
//         return "ra"; 
//     char *testString = (char *)malloc(10*sizeof(char));
//     sprintf(testString, "#%d", temporary);
//     return testString;
// }

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

ObjectCode *translateIRToObj(IntermediateRepresentation *ir, RegisterAssignment *rm, int includeASMComments){
    ObjectCode *currObj = NULL, *objCode = NULL;
    currObj = objCode;
    IRNode *p = ir->head;

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
                if (p->sourceKind == CONSTANT_SOURCE){
                    sprintf(currObj->assembly, "addi %s, %s, %d", 
                        getReg(rm, p->dest),
                        getReg(rm, p->source),
                        p->source2
                    );
                }
                else sprintf(currObj->assembly, "add %s, %s, %s", 
                    getReg(rm, p->dest),
                    getReg(rm, p->source),
                    getReg(rm, p->source2)
                );
                
                break;
            case MUL:
                sprintf(currObj->assembly, "mul %s, %s, %s", 
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
                    sprintf(currObj->assembly, "addi %s, zero, %d", 
                        getReg(rm, p->dest), 
                        p->source
                    );
                else if  (p->sourceKind == VARIABLE_SOURCE) {
                    sprintf(currObj->assembly, "addi %s, zero, %d", 
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
                break;
            case JUMP_REGISTER:
                sprintf(currObj->assembly, "jalr %s, 0(%s)", getReg(rm, RA_REGISTER), getReg(rm, p->dest));
                break;
            case MOV:
                sprintf(currObj->assembly, "mv %s, %s", getReg(rm, p->dest), getReg(rm, p->source));
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
                sprintf(currObj->assembly, "addi zero, zero, 0");
                break;
        }
        currObj->binary = asmToBinary(currObj->assembly);
        p = p->next;
    }

    return objCode;
}

