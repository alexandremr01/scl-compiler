#include "semantic_analysis.h"

void codeGenNode(ASTNode *node, IntermediateRepresentation *ir);

IntermediateRepresentation *codeGen(AbstractSyntaxTree *tree){
    IntermediateRepresentation *ir = newIntermediateRepresentation();

    codeGenNode(tree->root, ir);

    return ir;
}


void codeGenNode(ASTNode *node, IntermediateRepresentation *ir){
    if (node == NULL) return;
    SymbolicTableEntry * stEntry;
    int address_register;

    ASTNode *aux = node->firstChild;
    while (aux != NULL){
        codeGenNode(aux, ir);
        aux = aux->sibling;
    }

    switch (node->kind){
        case DECLARATION_NODE:
            break;
        case IF_NODE: 
            break;
        case WHILE_NODE: 
            break;
        case RETURN_NODE: 
            break;
        case ASSIGNMENT_NODE:
            if (node->firstChild->stEntry != NULL){
                addCommentIR(ir, "assignment");
                int address_register = ir->nextTempReg++, result_register  =  node->firstChild->sibling->tempRegResult;
                addLoadImIR(ir, address_register, node->firstChild->stEntry->address);
                addStoreIR(ir, address_register, 0, result_register);
                ir->nextTempReg++;
            }
            
            break;
        case CONSTANT_NODE:
            node->tempRegResult = ir->nextTempReg;
            addCommentIR(ir, "constant");
            addLoadImIR(ir, node->tempRegResult, atoi(node->name));
            ir->nextTempReg++;
            return;
        case CALL_NODE:
            break;
        case VAR_REFERENCE_NODE:
            address_register = ir->nextTempReg++;
            node->tempRegResult = ir->nextTempReg++;
            addCommentIR(ir, "var reference");
            addLoadImIR(ir, address_register, node->stEntry->address);
            addLoadMemIR(ir, node->tempRegResult, 0, address_register);      
            break;
    }
    return;
}
