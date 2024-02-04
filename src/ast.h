#include <stdlib.h>

typedef enum dataType {
    NONE_TYPE, INTEGER_TYPE, VOID_TYPE
} DataType;

typedef enum nodeKind {
    ROOT_NODE, DECLARATION_NODE, FUNCTION_DEFINITION_NODE, EMPTY_NODE,
    IF_NODE, WHILE_NODE, RETURN_NODE, EXPRESSION_NODE,
    ASSIGNMENT_NODE, SUM_NODE, MULTIPLICATION_NODE, CONSTANT_NODE, CALL_NODE,
    VAR_REFERENCE_NODE
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
ASTNode *newASTNode(NodeKind nodeKind);
ASTNode *newEmptyStatementNode();
void printTree(ASTNode *node, int level);
ASTNode *appendSibling(ASTNode *reference, ASTNode *newSibling);