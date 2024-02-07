#ifndef AST_H_
#define AST_H_

#include <stdlib.h>
#include "symbolic_table.h"
#include "datatypes.h"

typedef enum nodeKind {
    ROOT_NODE, DECLARATION_NODE, FUNCTION_DEFINITION_NODE, EMPTY_NODE,
    IF_NODE, WHILE_NODE, RETURN_NODE, EXPRESSION_NODE,
    ASSIGNMENT_NODE, SUM_NODE, MULTIPLICATION_NODE, CONSTANT_NODE, CALL_NODE,
    VAR_REFERENCE_NODE, COMPOUND_STATEMENT_NODE
} NodeKind;

typedef struct astNode {
    struct astNode *firstChild;
    struct astNode *sibling;

    SymbolicTableEntry *stEntry;

    DataType type;
    NodeKind kind;
    char *name;
    int line_number;
    int tempRegResult;
} ASTNode;

typedef struct abstractSyntaxTree {
    ASTNode *root;
    int nextTempReg;
} AbstractSyntaxTree;

AbstractSyntaxTree *newAbstractSyntaxTree();
ASTNode *newASTNode(NodeKind nodeKind);
ASTNode *newEmptyStatementNode();
void printTree(ASTNode *node, int level);
ASTNode *appendSibling(ASTNode *reference, ASTNode *newSibling);

#endif