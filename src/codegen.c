#include "semantic_analysis.h"

void codeGenNode(AbstractSyntaxTree *tree, ASTNode *node, IntermediateRepresentation *ir);

IntermediateRepresentation *codeGen(AbstractSyntaxTree *tree){
    IntermediateRepresentation *ir = newIntermediateRepresentation();

    codeGenNode(tree, tree->root, ir);

    return ir;
}


void codeGenNode(AbstractSyntaxTree *tree, ASTNode *node, IntermediateRepresentation *ir){
    if (node == NULL) return;
    SymbolicTableEntry * stEntry;
    int address_register;

    ASTNode *aux = node->firstChild;
    while (aux != NULL){
        codeGenNode(tree, aux, ir);
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
                int address_register = tree->nextTempReg++, result_register  =  node->firstChild->sibling->tempRegResult;
                addLoadImIR(ir, address_register, node->firstChild->stEntry->address);
                addStoreIR(ir, address_register, 0, result_register);
                tree->nextTempReg++;
            }
            
            break;
        case CONSTANT_NODE:
            node->tempRegResult = tree->nextTempReg;
            addCommentIR(ir, "constant");
            addLoadImIR(ir, node->tempRegResult, atoi(node->name));
            tree->nextTempReg++;
            return;
        case CALL_NODE:
            break;
        case VAR_REFERENCE_NODE:
            address_register = tree->nextTempReg++;
            node->tempRegResult = tree->nextTempReg++;
            addCommentIR(ir, "var reference");
            addLoadImIR(ir, address_register, node->stEntry->address);
            addLoadMemIR(ir, node->tempRegResult, 0, address_register);      
            break;
    }
    return;
}
