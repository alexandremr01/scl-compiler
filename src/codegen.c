#include "semantic_analysis.h"
#define INITIAL_STACK 2000

void codeGenNode(ASTNode *node, IntermediateRepresentation *ir, SymbolicTableGlobals *globals);

void call(IntermediateRepresentation *ir, SymbolicTableEntry *fn) {
    int address_register = ir->nextTempReg++;
    addLoadAddressIR(ir, address_register, fn);
    addJumpRegisterIR(ir, address_register);
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
    addJumpImIR(ir, ir->lastAddress);
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
    if (node->kind == FUNCTION_DEFINITION_NODE){
        ir->lastStackAddress = 0;
        addLabelIR(ir, node->stEntry);
    } 
    while (aux != NULL){
        codeGenNode(aux, ir, globals);
        aux = aux->sibling;
    }

    switch (node->kind){
        case DECLARATION_NODE:
            if (node->stEntry->scope_level == 0){
                SymbolicTableGlobals *next = globals->next;
                SymbolicTableGlobals *new_node = (SymbolicTableGlobals *) malloc(sizeof(SymbolicTableGlobals));
                new_node->next = next;
                new_node->entry = node->stEntry;
                globals->next = new_node;
            } else {
                node->stEntry->address = ir->lastStackAddress;
                ir->lastStackAddress+=4;
            }
            break;
        case FUNCTION_DEFINITION_NODE:
            address_register = ir->nextTempReg++;
            addLoadMemIR(ir, address_register, 0, -1);  
            addJumpRegisterIR(ir, address_register);
            ir->lastStackAddress = bkp_stack;
            break;
        case IF_NODE: 
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
            if (node->firstChild->stEntry != NULL){
                addCommentIR(ir, "assignment");
                int address_register = ir->nextTempReg++, result_register  =  node->firstChild->sibling->tempRegResult;
                addLoadAddressIR(ir, address_register, node->firstChild->stEntry);
                addStoreIR(ir, address_register, 0, result_register);
            }
            
            break;
        case CONSTANT_NODE:
            node->tempRegResult = ir->nextTempReg;
            addCommentIR(ir, "constant");
            addLoadImIR(ir, node->tempRegResult, atoi(node->name));
            ir->nextTempReg++;
            return;
        case CALL_NODE:
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
