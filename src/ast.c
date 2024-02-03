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
    }
    ASTNode *aux = node->firstChild;
    while (aux != NULL){
        printTree(aux, level+1);
        aux = aux->sibling;
    }
}
