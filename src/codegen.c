#include "semantic_analysis.h"

#define SP_REGISTER -1
#define A0_REGISTER -2
#define X0_REGISTER -3
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

    ASTNode *aux = node->firstChild;
    if (node->kind == FUNCTION_DEFINITION_NODE){
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
            }
            break;
        case FUNCTION_DEFINITION_NODE:
            // lw rx3, 0(sp)
            // jr rx3
            address_register = ir->nextTempReg++;
            addLoadMemIR(ir, address_register, 0, -1);  
            addJumpRegisterIR(ir, address_register);
            break;
        case IF_NODE: 
            break;
        case WHILE_NODE: 
            break;
        case RETURN_NODE: 
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
            addLoadAddressIR(ir, address_register, node->stEntry);
            addLoadMemIR(ir, node->tempRegResult, 0, address_register);      
            break;
    }
    return;
}
