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

