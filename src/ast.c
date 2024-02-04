#include "ast.h"
#include <stdio.h>

AbstractSyntaxTree *newAbstractSyntaxTree(ASTNode *root){
    AbstractSyntaxTree *ast = (AbstractSyntaxTree *)malloc(sizeof(AbstractSyntaxTree));
    ast->root = root;
    return ast;
}

ASTNode *newASTNode(NodeKind nodeKind){
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    node->firstChild = NULL;
    node->sibling = NULL;
    node->name = NULL;
    node->type = NONE_TYPE;
    node->kind = nodeKind;
    return node;
}

ASTNode *appendSibling(ASTNode *reference, ASTNode *newSibling){
    if (reference == NULL) 
        return newSibling;
    ASTNode *p = reference;
    while(p->sibling != NULL) 
        p = p->sibling;
    p->sibling = newSibling; 
    return reference;
}

void printTree(ASTNode *node, int level){
    if (node == NULL) return;
    for(int i=0; i<level;i++) printf("\t");
    switch (node->kind){
        case FUNCTION_DEFINITION_NODE:
            printf("Declaration: function %s with return type %d\n", node->name, node->type);
            break;
        case ROOT_NODE:
            printf("Program Root\n"); 
            break;
        case DECLARATION_NODE:
            printf("Declaration: var %s with type %d\n", node->name, node->type);
            break;
        case IF_NODE: 
            printf("If\n");
            break;
        case WHILE_NODE: 
            printf("While\n");
            break;
        case RETURN_NODE: 
            printf("Return\n");
            break;
        case EXPRESSION_NODE:
            printf("Expression: TBD\n");
            break;
        case ASSIGNMENT_NODE:
            printf("Assignment\n");
            break;
        case SUM_NODE:
            printf("Sum\n");
            break;
        case MULTIPLICATION_NODE:
            printf("Multiplication\n");
            break;
        case CONSTANT_NODE:
            printf("Constant %s\n", node->name);
            break;
        case CALL_NODE:
            printf("Call %s\n", node->name);
            break;
        case VAR_REFERENCE_NODE:
            printf("Variable %s\n", node->name);
            break;
    }
    ASTNode *aux = node->firstChild;
    while (aux != NULL){
        printTree(aux, level+1);
        aux = aux->sibling;
    }
}
