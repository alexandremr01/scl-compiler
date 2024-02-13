#include "semantic_analysis.h"
#define INITIAL_STACK 2000

void codeGenNode(ASTNode *node, IntermediateRepresentation *ir, SymbolicTableGlobals *globals);

void call(IntermediateRepresentation *ir, SymbolicTableEntry *fn) {
    int dest_address_register = ir->nextTempReg++;
    int current_address_register = RA_REGISTER;
    addJumpIR(ir, fn);
}

void genHeader(IntermediateRepresentation *ir, SymbolicTableEntry *main){
    for (int i=0; i<2; i++)
        addNopIR(ir);
    addAdditionImIR(ir, 
        SP_REGISTER, 
        X0_REGISTER,
        INITIAL_STACK
    );
    call(ir, main);
    for (int i=0; i<4; i++)
        addNopIR(ir);
    addJumpImIR(ir, -2);
}

IntermediateRepresentation *codeGen(AbstractSyntaxTree *tree){
    IntermediateRepresentation *ir = newIntermediateRepresentation();
    SymbolicTableGlobals *globals = (SymbolicTableGlobals *) malloc(sizeof(SymbolicTableGlobals));
    globals->next = NULL;
    genHeader(ir, tree->root->stEntry);
    codeGenNode(tree->root, ir, globals);

    ir->globals = globals;

    return ir;
}

void codeGenNode(ASTNode *node, IntermediateRepresentation *ir, SymbolicTableGlobals *globals){
    if (node == NULL) return;
    SymbolicTableEntry * stEntry;
    int address_register;
    int bkp_stack = ir->lastStackAddress;
    ASTNode *aux = node->firstChild;
    int returnStackPosition = 0;
    int localsSize = 0;
    if (node->kind == IF_NODE) {
        codeGenNode(node->firstChild, ir, globals);
        IRNode *endif = newIRNode(NOP);
        IRNode *bneq = addBNEQIR(ir, 
            node->firstChild->tempRegResult, 
            X0_REGISTER,
            0
        );
        codeGenNode(node->firstChild->sibling, ir, globals);
        addNode(ir, endif);
        bneq->imm = endif->address - bneq->address;
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
                localsSize += 4;
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
                localsSize += 4;
            }
            local = local->locals;
        }
    } 
    while (aux != NULL){
        codeGenNode(aux, ir, globals);
        aux = aux->sibling;
    }
    ASTNode *parameter;
    switch (node->kind){
        case DECLARATION_NODE:
            if (node->stEntry->scope_level == 0){
                SymbolicTableGlobals *next = globals->next;
                SymbolicTableGlobals *new_node = (SymbolicTableGlobals *) malloc(sizeof(SymbolicTableGlobals));
                new_node->next = next;
                new_node->entry = node->stEntry;
                globals->next = new_node;
            } 
            break;
        case FUNCTION_DEFINITION_NODE:
            address_register = ir->nextTempReg++;
            addLoadMemIR(ir, RA_REGISTER, returnStackPosition, SP_REGISTER); 
            addAdditionImIR(ir, SP_REGISTER, SP_REGISTER, localsSize);
            addJumpRegisterIR(ir, RA_REGISTER);
            ir->lastStackAddress = bkp_stack;
            break;
        case WHILE_NODE: 
            break;
        case RETURN_NODE: 
            addMovIR(ir, A0_REGISTER, node->firstChild->tempRegResult);
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
            
            if (node->firstChild->stEntry->scope_level == 0){
                int address_register = ir->nextTempReg++;
                addLoadAddressIR(ir, address_register, node->firstChild->stEntry);
                addStoreIR(ir, address_register, 0, result_register);
            } else {
                addStoreIR(ir, SP_REGISTER, node->firstChild->stEntry->address, result_register); 
            }
            
            break;
        case CONSTANT_NODE:
            node->tempRegResult = ir->nextTempReg;
            addCommentIR(ir, "constant");
            addLoadImIR(ir, node->tempRegResult, atoi(node->name));
            ir->nextTempReg++;
            return;
        // TODO: refactor all relational to use same function
        case LT_NODE:
            node->tempRegResult = ir->nextTempReg++;
            addLTIR(ir, 
                node->firstChild->tempRegResult, 
                node->firstChild->sibling->tempRegResult,
                node->tempRegResult
            );
            break;
        case GT_NODE:
            node->tempRegResult = ir->nextTempReg++;
            addGTIR(ir, 
                node->firstChild->tempRegResult, 
                node->firstChild->sibling->tempRegResult,
                node->tempRegResult
            );
            break;
        case LEQ_NODE:
            node->tempRegResult = ir->nextTempReg++;
            addLEQIR(ir, 
                node->firstChild->tempRegResult, 
                node->firstChild->sibling->tempRegResult,
                node->tempRegResult
            );
            break;
        case GEQ_NODE:
            node->tempRegResult = ir->nextTempReg++;
            addGEQIR(ir, 
                node->firstChild->tempRegResult, 
                node->firstChild->sibling->tempRegResult,
                node->tempRegResult
            );
            break;
        case EQ_NODE:
            node->tempRegResult = ir->nextTempReg++;
            int auxReg = ir->nextTempReg++;
            addSubtractionIR(ir, 
                node->firstChild->tempRegResult, 
                node->firstChild->sibling->tempRegResult,
                auxReg
            );
            addBEQIR(ir, 
                auxReg, 
                X0_REGISTER,
                8
            );
            addLoadImIR(ir, node->tempRegResult, 1);
            addJumpImIR(ir, 8);
            addLoadImIR(ir, node->tempRegResult, 0);
            break;
        case DIFF_NODE:
            node->tempRegResult = ir->nextTempReg++;
            addDiffIR(ir, 
                node->firstChild->tempRegResult, 
                node->firstChild->sibling->tempRegResult,
                node->tempRegResult
            );
            break;
        case CALL_NODE:
            parameter = node->firstChild;
            while (parameter != NULL){
                addAdditionImIR(ir, SP_REGISTER, SP_REGISTER, -4);
                addStoreIR(ir, SP_REGISTER, 0, parameter->tempRegResult); 
                parameter = parameter->sibling;
            }
            call(ir, node->stEntry);
            break;
        case VAR_REFERENCE_NODE:
            address_register = ir->nextTempReg++;
            node->tempRegResult = ir->nextTempReg++;
            addCommentIR(ir, "var reference");
            if (node->stEntry->scope_level == 0) { 
                //global variable: absolute address
                addLoadAddressIR(ir, address_register, node->stEntry);
                addLoadMemIR(ir, node->tempRegResult, 0, address_register);      
            } else { 
                // otherwise: wrt to SP
                addLoadMemIR(ir, node->tempRegResult, node->stEntry->address, SP_REGISTER); 
            }
            break;
    }
    return;
}
