#include "codegen.h"
#define INITIAL_STACK 2000

void codeGenNode(ASTNode*, IntermediateRepresentation*, IRNode*);

void genHeader(IntermediateRepresentation *ir, SymbolicTableEntry *main){
    for (int i=0; i<2; i++)
        addNopIR(ir);
    addAdditionImIR(ir, 
        SP_REGISTER, 
        X0_REGISTER,
        INITIAL_STACK
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
        addAdditionImIR(ir, SP_REGISTER, SP_REGISTER, -4);
        addStoreIR(ir, SP_REGISTER, 0, RA_REGISTER); 
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
        localsSize += 4; // skip return address
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
            address_register = ir->nextTempReg++;
            addNode(ir, functionEnd);
            addLoadMemIR(ir, RA_REGISTER, returnStackPosition, SP_REGISTER); 
            addAdditionImIR(ir, SP_REGISTER, SP_REGISTER, localsSize);
            addJumpRegisterIR(ir, RA_REGISTER);
            ir->lastStackAddress = bkp_stack;
            break;
        case RETURN_NODE: 
            addMovIR(ir, A0_REGISTER, node->firstChild->tempRegResult);
            addRelativeJump(ir, functionEnd);
            break;
        case MULTIPLICATION_NODE:
            node->tempRegResult = ir->nextTempReg++;
            addMultiplicationIR(ir, 
                node->firstChild->tempRegResult, 
                node->firstChild->sibling->tempRegResult,
                node->tempRegResult
            );
            break;

        case SUM_NODE:
            node->tempRegResult = ir->nextTempReg++;
            addAdditionIR(ir, 
                node->firstChild->tempRegResult, 
                node->firstChild->sibling->tempRegResult,
                node->tempRegResult
            );
            break;
        case SUBTRACTION_NODE:
            node->tempRegResult = ir->nextTempReg++;
            addSubtractionIR(ir, 
                node->firstChild->tempRegResult, 
                node->firstChild->sibling->tempRegResult,
                node->tempRegResult
            );
            break;
        case ASSIGNMENT_NODE:
            addCommentIR(ir, "assignment");
            int result_register  =  node->firstChild->sibling->tempRegResult;
            numElement = 1;
            if (node->firstChild->firstChild != NULL && node->firstChild->firstChild->kind == VAR_INDEXING_NODE) 
                numElement = atoi(node->firstChild->firstChild->name);
            offset = (numElement-1)*getSize(node->firstChild->stEntry->type);

            int address_register = ir->nextTempReg++;
            aux_register = ir->nextTempReg++;

            if (node->firstChild->stEntry->scope_level == 0 && node->firstChild->firstChild == NULL) { 
                // global non array variable 
                addGetPC(ir, address_register, 0);
                addAdditionImIR(ir, address_register, address_register, 8);
                addStoreVarAddress(ir, result_register, address_register, node->firstChild->stEntry);
            } else if (node->firstChild->stEntry->scope_level == 0 && node->firstChild->firstChild != NULL) { 
                // global array
                addLoadImIR(ir, address_register, 0);
                for (int i=0; i<getSize(node->firstChild->stEntry->type); i++)
                    addAdditionIR(ir, address_register, node->firstChild->firstChild->tempRegResult, address_register);
                addGetPC(ir, aux_register, 0);
                addAdditionImIR(ir, aux_register, aux_register, 12);
                addAdditionIR(ir, address_register, aux_register, address_register);
                addStoreVarAddress(ir, result_register, address_register, node->firstChild->stEntry);
            } else if (node->firstChild->stEntry->scope_level > 0 && node->firstChild->firstChild != NULL) { 
                // local array
                addLoadImIR(ir, address_register, node->firstChild->stEntry->address);
                for (int i=0; i<getSize(node->firstChild->stEntry->type); i++)
                    addAdditionIR(ir, address_register, node->firstChild->firstChild->tempRegResult, address_register);
                addAdditionIR(ir, address_register, SP_REGISTER, address_register);
                addStoreIR(ir, address_register, 0, result_register);
            } else { 
                // local non array
                addStoreIR(ir, SP_REGISTER, node->firstChild->stEntry->address+offset, result_register); 
            }
            
            break;
        case CONSTANT_NODE:
            node->tempRegResult = ir->nextTempReg++;
            addCommentIR(ir, "constant");
            addLoadImIR(ir, node->tempRegResult, atoi(node->name));
            return;
        case LT_NODE:
        case GT_NODE:
        case LEQ_NODE:
        case GEQ_NODE:
        case EQ_NODE:
        case DIFF_NODE:
            node->tempRegResult = ir->nextTempReg++;
            int auxReg = ir->nextTempReg++;
            addSubtractionIR(ir, 
                node->firstChild->tempRegResult, 
                node->firstChild->sibling->tempRegResult,
                auxReg
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
            break;
        case CALL_NODE:
            parameter = node->firstChild;
            while (parameter != NULL){
                addAdditionImIR(ir, SP_REGISTER, SP_REGISTER, -4);
                addStoreIR(ir, SP_REGISTER, 0, parameter->tempRegResult); 
                parameter = parameter->sibling;
            }
            addJumpIR(ir, node->stEntry);
            break;
        case ASM_NODE:
            addRawIR(ir, node->name);
            break;
        case VAR_REFERENCE_NODE:
            numElement = 1;
            if (node->firstChild != NULL && node->firstChild->kind == VAR_INDEXING_NODE) 
                numElement = atoi(node->firstChild->name);
            offset = (numElement-1)*getSize(node->stEntry->type);
            
            address_register = ir->nextTempReg++;
            node->tempRegResult = ir->nextTempReg++;
            aux_register = ir->nextTempReg++;
            addCommentIR(ir, "var reference");
            if (node->stEntry->scope_level == 0 && node->firstChild == NULL) { 
                // global non array variable 
                addGetPC(ir, address_register, 0);
                addAdditionImIR(ir, address_register, address_register, 8);
                addLoadVarAddress(ir, node->tempRegResult, address_register, node->stEntry);
            } else if (node->stEntry->scope_level == 0 && node->firstChild != NULL) { 
                // global array
                addLoadImIR(ir, address_register, 0);
                for (int i=0; i<getSize(node->stEntry->type); i++)
                    addAdditionIR(ir, address_register, node->firstChild->tempRegResult, address_register);
                addGetPC(ir, aux_register, 0);
                addAdditionImIR(ir, aux_register, aux_register, 12);
                addAdditionIR(ir, address_register, aux_register, address_register);
                addLoadVarAddress(ir, node->tempRegResult, address_register, node->stEntry);
            } else if (node->stEntry->scope_level > 0 && node->firstChild != NULL) { 
                // local array
                addLoadImIR(ir, address_register, node->stEntry->address);
                for (int i=0; i<getSize(node->stEntry->type); i++)
                    addAdditionIR(ir, address_register, node->firstChild->tempRegResult, address_register);
                addAdditionIR(ir, address_register, SP_REGISTER, address_register);
                addLoadMemIR(ir, node->tempRegResult, 0, address_register);
            } else { 
                // local non array
                addLoadMemIR(ir, node->tempRegResult, node->stEntry->address, SP_REGISTER); 
            }
            break;
    }
    return;
}


                