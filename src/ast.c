#include "ast.h"
#include <stdio.h>

AbstractSyntaxTree *newAbstractSyntaxTree(ASTNode *root){
    AbstractSyntaxTree *ast = (AbstractSyntaxTree *)malloc(sizeof(AbstractSyntaxTree));
    ast->root = root;
    return ast;
}

ASTNode *newASTNode(){
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    node->firstChild = NULL;
    node->sibling = NULL;
    node->name = NULL;
    node->type = NONE_TYPE;
    return node;
}

void printTree(ASTNode *node, int level){
    if (node == NULL) return;
    for(int i=0; i<level;i++) printf("\t");
    switch (node->kind){
        case ROOT_NODE:
            printf("Program Root\n"); break;
        case DECLARATION_NODE:
            printf("Declaration: var %s with type %d\n", node->name, node->type);
    }
    ASTNode *aux = node->firstChild;
    while (aux != NULL){
        printTree(aux, level+1);
        aux = aux->sibling;
    }
}
