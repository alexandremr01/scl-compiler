#include <stdlib.h>

typedef enum dataType {
    NONE_TYPE, INTEGER_TYPE, VOID_TYPE
} DataType;

typedef enum nodeKind {
    ROOT_NODE, DECLARATION_NODE
} NodeKind;

typedef struct astNode {
    struct astNode *firstChild;
    struct astNode *sibling;

    DataType type;
    NodeKind kind;
    char *name;
} ASTNode;

typedef struct abstractSyntaxTree {
    ASTNode *root;
} AbstractSyntaxTree;

AbstractSyntaxTree *newAbstractSyntaxTree();
ASTNode *newASTNode();
void printTree(ASTNode *node, int level);