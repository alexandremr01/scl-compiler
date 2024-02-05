#include "semantic_analysis.h"

void semanticAnalysisNode(ASTNode *node, SymbolicTable* symbolicTable, int debug, int *errors, int scope_level);

int semanticAnalysis(AbstractSyntaxTree *tree, SymbolicTable* symbolicTable, int debug){
    int errors = 0;
    semanticAnalysisNode(tree->root, symbolicTable, debug, &errors, 0);

    SymbolicTableEntry *mainFunction = getSymbolicTableEntry(symbolicTable, "main");
    if (mainFunction == NULL || mainFunction->kind != FUNCTION_ENTRY) {
        printf("Error: No function named main.\n");
        errors++;
    }
    return errors;
}

void semanticAnalysisNode(ASTNode *node, SymbolicTable* symbolicTable, int debug, int *errors, int scope_level){
    if (node == NULL) return;
    SymbolicTableEntry * stEntry;
    switch (node->kind){
        case FUNCTION_DEFINITION_NODE:
            if (debug) printf("Declaration: function %s with return type %d\n", node->name, node->type);
            insertSymbolicTable(symbolicTable, node->name, FUNCTION_ENTRY, node->type, node->line_number, scope_level);
            scope_level += 1;
            break;
        case ROOT_NODE:
            if (debug) printf("Program Root\n"); 
            break;
        case DECLARATION_NODE:
            if (debug) printf("Declaration: var %s with type %d\n", node->name, node->type);
            if (node->type == VOID_TYPE) {
                printf("Line %d: Variable \'%s\' declared void.\n", node->line_number, node->name);
                *errors += 1;
            }
            stEntry = getSymbolicTableEntry(symbolicTable, node->name);
            if (stEntry != NULL && stEntry->scope_level == scope_level){
                printf("Line %d: Name \'%s\' already in use. First defined in line %d.\n", node->line_number, node->name, stEntry->definition_line_number);
                *errors += 1;
            }
            if (node->type != VOID_TYPE && (stEntry == NULL || stEntry->scope_level < scope_level))
                insertSymbolicTable(symbolicTable, node->name, VARIABLE_ENTRY, node->type, node->line_number, scope_level);
            
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
            stEntry = getSymbolicTableEntry(symbolicTable, node->name);
            if (stEntry == NULL){
                printf("Line %d: Name \'%s\' does not exist.\n", node->line_number,node->name);
                *errors += 1;
            } else if (stEntry->kind != FUNCTION_ENTRY){
                printf("Line %d: Name \'%s\' is not a function.\n", node->line_number, node->name);
                *errors += 1;
            }

            break;
        case VAR_REFERENCE_NODE:
            if (debug) printf("Variable %s\n", node->name);
            stEntry = getSymbolicTableEntry(symbolicTable, node->name);
            if (stEntry == NULL){
                printf("Line %d: Name \'%s\' is not defined.\n", node->line_number, node->name);
                *errors += 1;
            }// else if (stEntry->)

            break;
    }
    ASTNode *aux = node->firstChild;
    while (aux != NULL){
        semanticAnalysisNode(aux, symbolicTable, debug, errors, scope_level);
        aux = aux->sibling;
    }
}
