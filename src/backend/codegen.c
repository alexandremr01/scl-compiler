#include "codegen.h"
#define INITIAL_STACK 2000
void codeGenNode(ASTNode*, IntermediateRepresentation*, IRNode*);

void enableFloatingPoint(IntermediateRepresentation *ir){
    addLoadUpperImIR(ir, T0_REGISTER, 0x6000>>12);
    addCSRReadWrite(ir, X0_REGISTER, 0x300, T0_REGISTER);
}

int savedRegisters[SAVED_REGISTERS] = {
    -6, -7, -8, -29, -30, -31, -32, -13, -14, -15, -16, -17, -18
};

int savedFloatingPointRegisters[SAVED_REGISTERS] = {
    -1, -2, -3, -4, -5, -6, -7, -8, -12,
    -29, -30, -31, -32
};

void genHeader(IntermediateRepresentation *ir, SymbolicTableEntry *main){
    for (int i=0; i<2; i++)
        addNopIR(ir);
    enableFloatingPoint(ir);

    addPrepareStack(ir);

    addAdditionImIR(ir, 
        SP_REGISTER, 
        SP_REGISTER,
        -184
    );
    addJumpIR(ir, main);
    for (int i=0; i<4; i++)
        addNopIR(ir);
    addJumpImIR(ir, -8);
}

typedef IRNode* (*comparisonFunction)(IntermediateRepresentation*, int, int, int);

// Function that takes an int and returns a pointer to a function with the signature of addBEQIR
comparisonFunction getComparisonFunction(NodeKind instruction) {
    switch (instruction) {
        case LT_NODE:
            return addBLTIR;
        case GT_NODE:
            return addBGTIR;
        case LEQ_NODE:
            return addBLEIR;
        case GEQ_NODE:
            return addBGEIR;
        case EQ_NODE:
            return addBEQIR;
        case DIFF_NODE:
            return addBNEQIR;
        default:
            return NULL;
    }
}

comparisonFunction getFloatComparisonFunction(NodeKind instruction) {
    switch (instruction) {
        case LT_NODE:
            return addFloatLT;
        case GT_NODE:
            return addFloatGT;
        case LEQ_NODE:
            return addFloatLEQ;
        case GEQ_NODE:
            return addFloatGEQ;
        default:
            return NULL;
    }
}


IntermediateRepresentation *codeGen(AbstractSyntaxTree *tree){
    IntermediateRepresentation *ir = newIntermediateRepresentation();
 
    genHeader(ir, tree->root->stEntry);
    codeGenNode(tree->root, ir, 0);

    return ir;
}

void codeGenNode(ASTNode *node, IntermediateRepresentation *ir, IRNode *functionEnd){
    if (node == NULL) return;
    SymbolicTableEntry * stEntry;
    int address_register;
    int aux_register;
    int numElement = 1;
    int offset = 0;
    int bkp_stack = ir->lastStackAddress;
    ASTNode *aux = node->firstChild;
    int returnStackPosition = 0;
    int localsSize = 0;
    if (node->kind == FUNCTION_DECLARATION_NODE) return;
    if (node->kind == NEG_NODE) {
        node->tempRegResult = registerNewTemporary(ir, node->type == FLOAT_TYPE);
        codeGenNode(node->firstChild, ir, functionEnd);
        if (node->type == FLOAT_TYPE) 
            addFSGNJN(ir, node->firstChild->tempRegResult, node->firstChild->tempRegResult, node->tempRegResult);
        else addSubtractionIR(ir, 
            X0_REGISTER, 
            node->firstChild->tempRegResult,
            node->tempRegResult,
            0
        );
        return;
    }
    if (node->kind == IF_NODE) {
        codeGenNode(node->firstChild, ir, functionEnd);
        IRNode *endif = newIRNode(NOP);
        IRNode *bneq = addBNEQIR(ir, 
            node->firstChild->tempRegResult, 
            X0_REGISTER,
            0
        );
        codeGenNode(node->firstChild->sibling, ir, functionEnd);
        
        // if there is an else
        ASTNode *elseNode = node->firstChild->sibling->sibling;
        if (elseNode != NULL) {
            IRNode *endelse = newIRNode(NOP);
            IRNode *jumpelse = addJumpImIR(ir, 4);
            addNode(ir, endif);
            codeGenNode(elseNode, ir, functionEnd);
            addNode(ir, endelse);
            jumpelse->source = endelse->address - jumpelse->address;
        } else addNode(ir, endif);
        bneq->imm = endif->address - bneq->address;
        return;
    }
    if (node->kind == WHILE_NODE) {
        IRNode *startWhile = newIRNode(NOP);
        addNode(ir, startWhile);
        addCommentIR(ir, "condition");

        codeGenNode(node->firstChild, ir, functionEnd);

        IRNode *endwhile = newIRNode(JUMP);
        endwhile->sourceKind = CONSTANT_SOURCE;
        addCommentIR(ir, "jump if false");

        IRNode *bneq = addBNEQIR(ir, 
            node->firstChild->tempRegResult, 
            X0_REGISTER,
            0
        );
        addCommentIR(ir, "while body");

        codeGenNode(node->firstChild->sibling, ir, functionEnd);
        addCommentIR(ir, "return to condition");
        addNode(ir, endwhile);
        // goes to next instruction after the endwhile
        bneq->imm = endwhile->address - bneq->address + 4;
        endwhile->source = startWhile->address - endwhile->address; // problem: has to go back to beggining of comparison

        return;
    }
    if (node->kind == FUNCTION_DEFINITION_NODE){
        ir->lastStackAddress = 0;
        addLabelIR(ir, node->stEntry);
        addAdditionImIR(ir, SP_REGISTER, SP_REGISTER, -4*(SAVED_REGISTERS+1));
        addStoreIR(ir, SP_REGISTER, 4*SAVED_REGISTERS, RA_REGISTER, 0); 
        // store preserved registers
        int reg = 0;
        while (reg < SAVED_REGISTERS) {
            if (reg < SAVED_REGISTERS/2)
                addStoreIR(ir, SP_REGISTER, 4*(SAVED_REGISTERS-1-reg), savedRegisters[reg], 0);
            else addStoreIR(ir, SP_REGISTER, 4*(SAVED_REGISTERS-1-reg), savedFloatingPointRegisters[reg-SAVED_REGISTERS/2], 1);
            reg += 1;
        } 
        // first, iterate over non-parameter locals
        SymbolicTableEntry *local = node->stEntry->locals;
        while(local != NULL) {
            if (!local->isParameter) {
                local->address = localsSize;
                localsSize += getSize(local->type)*local->numElements;
            }
            local = local->locals;
        }
        addAdditionImIR(ir, SP_REGISTER, SP_REGISTER, -localsSize);
        returnStackPosition = localsSize;
        localsSize += 4*(SAVED_REGISTERS+1); // skip return address and registers t0-t6
        // now, iterate over parameters to give their addresses
        local = node->stEntry->locals;
        while(local != NULL) {
            if (local->isParameter) {
                local->address = localsSize;
                localsSize += getSize(local->type)*local->numElements;
            }
            local = local->locals;
        }
        functionEnd = newIRNode(NOP);
    } 
    while (aux != NULL){
        codeGenNode(aux, ir, functionEnd);
        aux = aux->sibling;
    }
    ASTNode *parameter;
    switch (node->kind){
        case FUNCTION_DEFINITION_NODE:
            address_register = registerNewTemporary(ir, 0);
            addNode(ir, functionEnd);

            int reg = SAVED_REGISTERS-1;
            while (reg >= 0) {
                if (reg < SAVED_REGISTERS/2)
                    addLoadMemIR(ir, savedRegisters[reg], returnStackPosition + 4*(SAVED_REGISTERS-1-reg), SP_REGISTER, 0); 
                else addLoadMemIR(ir, savedFloatingPointRegisters[reg-SAVED_REGISTERS/2], returnStackPosition + 4*(SAVED_REGISTERS-1-reg), SP_REGISTER, 1);                 
                reg -= 1;
            } 
            
            addLoadMemIR(ir, RA_REGISTER, returnStackPosition + 4*SAVED_REGISTERS, SP_REGISTER, 0); 
            addAdditionImIR(ir, SP_REGISTER, SP_REGISTER, localsSize);
            addJumpRegisterIR(ir, RA_REGISTER);
            ir->lastStackAddress = bkp_stack;
            break;
        case RETURN_NODE: 
            if (node->firstChild != NULL){ // there is return value
                if (node->firstChild->type == INTEGER_TYPE)
                    addMovIR(ir, A0_REGISTER, node->firstChild->tempRegResult);
                else if (node->firstChild->type == FLOAT_TYPE) 
                    addFloatMovIR(ir, FA0_REGISTER, node->firstChild->tempRegResult);
            }
            addRelativeJump(ir, functionEnd);
            break;
        case MULTIPLICATION_NODE:
            node->tempRegResult = registerNewTemporary(ir, 0);
            addMultiplicationIR(ir, 
                node->firstChild->tempRegResult, 
                node->firstChild->sibling->tempRegResult,
                node->tempRegResult, 
                node->type == FLOAT_TYPE
            );
            break;
        case SUM_NODE:
            node->tempRegResult = registerNewTemporary(ir, 0);
            addAdditionIR(ir, 
                node->firstChild->tempRegResult, 
                node->firstChild->sibling->tempRegResult,
                node->tempRegResult, 
                node->type == FLOAT_TYPE
            );
            break;
        case SUBTRACTION_NODE:
            node->tempRegResult = registerNewTemporary(ir, 0);
            addSubtractionIR(ir, 
                node->firstChild->tempRegResult, 
                node->firstChild->sibling->tempRegResult,
                node->tempRegResult, 
                node->type == FLOAT_TYPE
            );
            break;
        case ASSIGNMENT_NODE:
            addCommentIR(ir, "assignment");
            int result_register  =  node->firstChild->sibling->tempRegResult;
            numElement = 1;
            if (node->firstChild->firstChild != NULL && node->firstChild->firstChild->kind == VAR_INDEXING_NODE) 
                numElement = atoi(node->firstChild->firstChild->name);
            offset = (numElement-1)*getSize(node->firstChild->stEntry->type);

            int address_register = registerNewTemporary(ir, 0);
            aux_register = registerNewTemporary(ir, 0);

            int isFloat = node->firstChild->stEntry->type==FLOAT_TYPE;

            if (node->firstChild->stEntry->scope_level == 0 && node->firstChild->firstChild == NULL) { 
                // global non array variable 
                addGetPCVarAddress(ir, address_register, node->firstChild->stEntry);
                addAdditionImIR(ir, address_register, address_register, 8);
                addStoreVarAddress(ir, result_register, address_register, node->firstChild->stEntry, isFloat);
            } else if (node->firstChild->stEntry->scope_level == 0 && node->firstChild->firstChild != NULL) { 
                // global array
                addLoadImIR(ir, address_register, 0);
                for (int i=0; i<getSize(node->firstChild->stEntry->type); i++)
                    addAdditionIR(ir, address_register, node->firstChild->firstChild->tempRegResult, address_register, 0);
                addGetPCVarAddress(ir, aux_register, node->firstChild->stEntry);
                addAdditionImIR(ir, aux_register, aux_register, 12);
                addAdditionIR(ir, address_register, aux_register, address_register, 0);
                addStoreVarAddress(ir, result_register, address_register, node->firstChild->stEntry, isFloat);
            } else if (node->firstChild->stEntry->scope_level > 0 && node->firstChild->firstChild != NULL) { 
                // local array
                addLoadImIR(ir, address_register, node->firstChild->stEntry->address);
                for (int i=0; i<getSize(node->firstChild->stEntry->type); i++)
                    addAdditionIR(ir, address_register, node->firstChild->firstChild->tempRegResult, address_register, 0);
                addAdditionIR(ir, address_register, SP_REGISTER, address_register, 0);
                addStoreIR(ir, address_register, 0, result_register, isFloat);
            } else { 
                // local non array
                addStoreIR(ir, SP_REGISTER, node->firstChild->stEntry->address+offset, result_register, isFloat); 
            }
            
            break;
        case CONSTANT_NODE:
            aux_register = registerNewTemporary(ir, 0);
            node->tempRegResult = registerNewTemporary(ir, node->type == FLOAT_TYPE);
            addCommentIR(ir, "constant");
            if (node->type == FLOAT_TYPE) {
                float floatVal = strtof(node->name, NULL);
                union {
                    float f;
                    int i;
                } u;
                u.f = floatVal;
                addLoadUpperImIR(ir, aux_register, u.i >> 12);
                addAdditionImIR(ir, aux_register, aux_register, u.i & 0xFFF);
                addMovFromIntegerToFloat(ir, node->tempRegResult, aux_register);
            } else addLoadImIR(ir, node->tempRegResult, atoi(node->name));
            return;
        case LT_NODE:
        case GT_NODE:
        case LEQ_NODE:
        case GEQ_NODE:
        case EQ_NODE:
        case DIFF_NODE:
            node->tempRegResult = registerNewTemporary(ir, 0);
            if (node->firstChild->type == INTEGER_TYPE){
                int auxReg = registerNewTemporary(ir, 0);
                addSubtractionIR(ir, 
                    node->firstChild->tempRegResult, 
                    node->firstChild->sibling->tempRegResult,
                    auxReg,
                    0
                );
                getComparisonFunction(node->kind)(
                    ir, 
                    auxReg, 
                    X0_REGISTER,
                    12
                );
                addLoadImIR(ir, node->tempRegResult, 1);
                addJumpImIR(ir, 8);
                addLoadImIR(ir, node->tempRegResult, 0);
            } else {
                getFloatComparisonFunction(node->kind)(
                    ir, 
                    node->firstChild->tempRegResult, 
                    node->firstChild->sibling->tempRegResult,
                    node->tempRegResult
                );
            }
            break;
        case CALL_NODE:
            parameter = node->firstChild;
            while (parameter != NULL){
                addAdditionImIR(ir, SP_REGISTER, SP_REGISTER, -4);
                addStoreIR(ir, SP_REGISTER, 0, parameter->tempRegResult, parameter->type==FLOAT_TYPE); 
                parameter = parameter->sibling;
            }
            addJumpIR(ir, node->stEntry);
            node->tempRegResult = registerNewTemporary(ir, node->type==FLOAT_TYPE);
            if (node->type==FLOAT_TYPE)
                addFloatMovIR(ir, node->tempRegResult, FA0_REGISTER);
            else addMovIR(ir, node->tempRegResult, A0_REGISTER);
            break;
        case ASM_NODE:
            addRawIR(ir, node->name);
            break;
        case VAR_REFERENCE_NODE:
            numElement = 1;
            if (node->firstChild != NULL && node->firstChild->kind == VAR_INDEXING_NODE) 
                numElement = atoi(node->firstChild->name);
            offset = (numElement-1)*getSize(node->stEntry->type);
            
            address_register = registerNewTemporary(ir, 0);
            node->tempRegResult = registerNewTemporary(ir, 0);
            aux_register = registerNewTemporary(ir, 0);
            addCommentIR(ir, "var reference");
            if (node->stEntry->scope_level == 0 && node->firstChild == NULL) { 
                // global non array variable 
                addGetPCVarAddress(ir, address_register, node->stEntry);
                addAdditionImIR(ir, address_register, address_register, 8);
                addLoadVarAddress(ir, node->tempRegResult, address_register, node->stEntry, node->type == FLOAT_TYPE);
            } else if (node->stEntry->scope_level == 0 && node->firstChild != NULL) { 
                // global array
                addLoadImIR(ir, address_register, 0); 
                for (int i=0; i<getSize(node->stEntry->type); i++)
                    addAdditionIR(ir, address_register, node->firstChild->tempRegResult, address_register, 0);
                addGetPCVarAddress(ir, aux_register, node->stEntry);
                addAdditionImIR(ir, aux_register, aux_register, 12);
                addAdditionIR(ir, address_register, aux_register, address_register, 0);
                addLoadVarAddress(ir, node->tempRegResult, address_register, node->stEntry, node->type == FLOAT_TYPE);
            } else if (node->stEntry->scope_level > 0 && node->firstChild != NULL) { 
                // local array
                addLoadImIR(ir, address_register, node->stEntry->address);
                for (int i=0; i<getSize(node->stEntry->type); i++)
                    addAdditionIR(ir, address_register, node->firstChild->tempRegResult, address_register, 0);
                addAdditionIR(ir, address_register, SP_REGISTER, address_register, 0);
                addLoadMemIR(ir, node->tempRegResult, 0, address_register, node->type == FLOAT_TYPE);
            } else { 
                // local non array
                addLoadMemIR(ir, node->tempRegResult, node->stEntry->address, SP_REGISTER, node->type == FLOAT_TYPE); 
            }
            break;
        case DOT_PRODUCT_NODE:
            node->tempRegResult = registerNewTemporary(ir, 1);
            int aux_register_pa = registerNewTemporary(ir, 0);
            int aux_register = registerNewTemporary(ir, 0);
            int aux_register_current_a = registerNewTemporary(ir, 0);
            int aux_register_current_b = registerNewTemporary(ir, 0);
            addMovIR(ir, aux_register_current_a, node->firstChild->tempRegResult);
            addMovIR(ir, aux_register_current_b, node->firstChild->sibling->tempRegResult);
            // store the address of the first variable in 0x10000
            addLoadUpperImIR(ir, aux_register_pa, 0x10000 >> 12);
            addAdditionImIR(ir, aux_register_pa, aux_register_pa, 0x10000 & ((1 << 12) - 1));
            addStoreIR(ir, aux_register_pa, 0, node->firstChild->tempRegResult, 0);
            // store the address of the sedon variable in 0x10004
            addStoreIR(ir, aux_register_pa, 4, node->firstChild->sibling->tempRegResult, 0);
            // set PA to 0x100b0 and PB to 0x100b4

            // clear accumulator
            addMACCStore(ir, 0x10008);

            // call accelerator
            int first_child_size = node->firstChild->stEntry->numElements;
            int second_child_size = node->firstChild->sibling->stEntry->numElements;
            int vector_size = (first_child_size < second_child_size)? first_child_size : second_child_size;
            int num_iterations = (int) vector_size / 2;
            addLoadImIR(ir, aux_register, num_iterations);
            IRNode* macc_node = addSetPA(ir, 0x10000);
            addSetPB(ir, 0x10004);
            addMACC(ir);
            addAdditionImIR(ir, aux_register_current_a, aux_register_current_a, 8);
            addAdditionImIR(ir, aux_register_current_b, aux_register_current_b, 8);
            addStoreIR(ir, aux_register_pa, 0, aux_register_current_a, 0);
            addStoreIR(ir, aux_register_pa, 4, aux_register_current_b, 0);
            addAdditionImIR(ir, aux_register, aux_register, -1);
            addBEQIR(ir, aux_register, X0_REGISTER, 8);
            IRNode* n = addJumpImIR(ir, 0);
            n->source = macc_node->address - n->address; 
            
            addMACCStore(ir, 0x10008);
            addStoreIR(ir, aux_register_pa, 0, node->firstChild->tempRegResult, 0);
            // store result in a register
            addLoadUpperImIR(ir, aux_register, 0x10008 >> 12);
            addAdditionImIR(ir, aux_register, aux_register, 0x10008 & ((1 << 12) - 1));
            addLoadMemIR(ir, node->tempRegResult, 0, aux_register, 1);
            // Last element
            if (vector_size % 2 != 0) {
                int floatAuxRegister1 = registerNewTemporary(ir, 1);
                int floatAuxRegister2 = registerNewTemporary(ir, 1);
                addLoadMemIR(ir, floatAuxRegister1, 0, aux_register_current_a, 1);
                addLoadMemIR(ir, floatAuxRegister2, 0, aux_register_current_b, 1);
                addMultiplicationIR(ir, floatAuxRegister1, floatAuxRegister2, floatAuxRegister2, 1);
                addAdditionIR(ir, node->tempRegResult, floatAuxRegister2, node->tempRegResult, 1);
            }
            break;
        case REFERENCE_NODE: // similar to previous, but does not load
            numElement = 1;
            ASTNode *varNode = node->firstChild;
            if (varNode->firstChild != NULL && varNode->firstChild->kind == VAR_INDEXING_NODE) 
                numElement = atoi(varNode->firstChild->name);
            offset = (numElement-1)*getSize(node->stEntry->type);
            
            node->tempRegResult = registerNewTemporary(ir, 0);
            aux_register = registerNewTemporary(ir, 0);
            addCommentIR(ir, "reference");
            if (node->stEntry->scope_level == 0 && varNode->firstChild == NULL) { 
                // global non array variable 
                IRNode *n = addGetPCVarAddress(ir, node->tempRegResult, node->stEntry);
                addSumAddressDistance(ir, node->tempRegResult, node->stEntry, 4);
            } else if (node->stEntry->scope_level == 0 && varNode->firstChild != NULL) { 
                // global array
                addLoadImIR(ir, aux_register, 0); 
                for (int i=0; i<getSize(node->stEntry->type); i++)
                    addAdditionIR(ir, aux_register, varNode->firstChild->tempRegResult, aux_register, 0);
                IRNode *n = addGetPCVarAddress(ir, node->tempRegResult, node->stEntry);
                addAdditionIR(ir, node->tempRegResult, aux_register, node->tempRegResult, 0);
                addSumAddressDistance(ir, node->tempRegResult, node->stEntry, 8);
            } 
            else if (node->stEntry->scope_level > 0 && varNode->firstChild != NULL) { 
                // local array
                address_register = registerNewTemporary(ir, 0);
                addLoadImIR(ir, address_register, node->stEntry->address);
                for (int i=0; i<getSize(node->stEntry->type); i++)
                    addAdditionIR(ir, address_register, varNode->firstChild->tempRegResult, address_register, 0);
                addAdditionIR(ir, SP_REGISTER, address_register, node->tempRegResult, 0);
            } else { 
                // local non array
                addAdditionImIR(ir, node->tempRegResult, SP_REGISTER, node->stEntry->address); 
            }
            break;
    }
    return;
}

//erro em 39050
// 2.17958 to 2.63081
// 