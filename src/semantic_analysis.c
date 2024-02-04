#include "semantic_analysis.h"

void semanticAnalysisNode(ASTNode *node, SymbolicTable* symbolicTable, int debug, int *errors);

int semanticAnalysis(AbstractSyntaxTree *tree, SymbolicTable* symbolicTable, int debug){
    int errors = 0;
    semanticAnalysisNode(tree->root, symbolicTable, debug, &errors);
    return errors;
}

void semanticAnalysisNode(ASTNode *node, SymbolicTable* symbolicTable, int debug, int *errors){
    if (node == NULL) return;
    SymbolicTableEntry * stEntry;
    switch (node->kind){
        case FUNCTION_DEFINITION_NODE:
            if (debug) printf("Declaration: function %s with return type %d\n", node->name, node->type);
            break;
        case ROOT_NODE:
            if (debug) printf("Program Root\n"); 
            break;
        case DECLARATION_NODE:
            if (debug) printf("Declaration: var %s with type %d\n", node->name, node->type);
            if (node->type == VOID_TYPE) {
                printf("Error: Variable \'%s\' declared void.\n", node->name);
                *errors += 1;
            }
            stEntry = getVariable(symbolicTable, node->name);
            if (stEntry != NULL){
                printf("Error: Name \'%s\' already in use.\n", node->name);
                *errors += 1;
            }
            if (node->type != VOID_TYPE && stEntry == NULL)
                insertVariable(symbolicTable, node->name, node->type);
            
            break;
        case IF_NODE: 
            if (debug) printf("If\n");
            break;
        case WHILE_NODE: 
            if (debug) printf("While\n");
            break;
        case RETURN_NODE: 
            if (debug) printf("Return\n");
            break;
        case EXPRESSION_NODE:
            if (debug) printf("Expression: TBD\n");
            break;
        case ASSIGNMENT_NODE:
            if (debug) printf("Assignment\n");
            break;
        case SUM_NODE:
            if (debug) printf("Sum\n");
            break;
        case MULTIPLICATION_NODE:
            if (debug) printf("Multiplication\n");
            break;
        case CONSTANT_NODE:
            if (debug) printf("Constant %s\n", node->name);
            break;
        case CALL_NODE:
            if (debug) printf("Call %s\n", node->name);
            break;
        case VAR_REFERENCE_NODE:
            if (debug) printf("Variable %s\n", node->name);
            stEntry = getVariable(symbolicTable, node->name);
            if (stEntry == NULL){
                printf("Error: Name \'%s\' is not defined.\n", node->name);
                *errors += 1;
            }// else if (stEntry->)

            break;
    }
    ASTNode *aux = node->firstChild;
    while (aux != NULL){
        semanticAnalysisNode(aux, symbolicTable, debug, errors);
        aux = aux->sibling;
    }
}
