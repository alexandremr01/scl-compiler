#include "semantic_analysis.h"


typedef struct deleteStack {
    char *name;
    struct deleteStack *next;
} DeleteStack;

void addDeleteStack(DeleteStack *s, char *name) {
    DeleteStack *entry = (DeleteStack *)malloc(sizeof(DeleteStack));
    entry->name = name;
    entry->next = s->next;
    s->next = entry;
}
DeleteStack * newDeleteStack(){
    DeleteStack *stack = (DeleteStack *)malloc(sizeof(DeleteStack));
    stack->next = NULL;
    return stack;
}
void semanticAnalysisNode(ASTNode *node, SymbolicTable* symbolicTable, int debug, int *errors, int scope_level, DeleteStack *stack);

int semanticAnalysis(AbstractSyntaxTree *tree, SymbolicTable* symbolicTable, int debug){
    int errors = 0;

    DeleteStack *stack = newDeleteStack();
    semanticAnalysisNode(tree->root, symbolicTable, debug, &errors, 0, stack);

    SymbolicTableEntry *mainFunction = getSymbolicTableEntry(symbolicTable, "main");
    if (mainFunction == NULL || mainFunction->kind != FUNCTION_ENTRY) {
        printf("Error: No function named main.\n");
        errors++;
    }
    return errors;
}


void semanticAnalysisNode(ASTNode *node, SymbolicTable* symbolicTable, int debug, int *errors, int scope_level, DeleteStack *stack){
    if (node == NULL) return;
    SymbolicTableEntry * stEntry;
    if (node->kind == FUNCTION_DEFINITION_NODE) {
        if (debug) printf("Declaration: function %s with return type %d\n", node->name, node->type);
        stack = newDeleteStack();
        ASTNode *parameters = node->firstChild->sibling;
    
        ASTNode *p = parameters;
        DataTypeList *head=NULL, *list = NULL;
        while(p!=NULL){
            if (list == NULL){
                list = (DataTypeList *)malloc(sizeof(DataTypeList));
                list->next=NULL;
                head = list;
            } else {
                list->next = (DataTypeList *)malloc(sizeof(DataTypeList));
                list = list->next;
            }
            list->type = p->type;
            p = p->sibling;
        }

        insertFunction(
            symbolicTable, 
            node->name, 
            node->type, 
            node->line_number, 
            scope_level,
            head
        );
        scope_level++;
        while (parameters != NULL){
            semanticAnalysisNode(parameters, symbolicTable, debug, errors, scope_level, stack);
            parameters = parameters->sibling;
        }
        semanticAnalysisNode(node->firstChild, symbolicTable, debug, errors, scope_level, stack);

        scope_level--; 
        while (stack->next != NULL){
            removeSymbolicTableEntry(symbolicTable, stack->next->name);
            stack = stack->next;
        }
        return;
    }

    if (node->kind == CALL_NODE) {
        if (debug) printf("Call %s\n", node->name);
        stEntry = getSymbolicTableEntry(symbolicTable, node->name);
        if (stEntry == NULL){
            printf("Line %d: Name \'%s\' does not exist.\n", node->line_number,node->name);
            *errors += 1;
        } else if (stEntry->kind != FUNCTION_ENTRY){
            printf("Line %d: Name \'%s\' is not a function.\n", node->line_number, node->name);
            *errors += 1;
        } 
        node->type = stEntry->type;
        ASTNode *parameter = node->firstChild;
        DataTypeList *dtypes = stEntry->parameterTypes;

        int callErrors = 0, num_parameters=0, num_expected=0;
        while (parameter != NULL && dtypes != NULL){
            semanticAnalysisNode(parameter, symbolicTable, debug, errors, scope_level, stack);
            if (dtypes->type != parameter->type) {
                printf("Line %d: Parameter #%d of incorrect type. Has type %s, expected %s.\n", parameter->line_number, num_parameters, printType(parameter->type), printType(dtypes->type));
                *errors += 1;
                callErrors += 1;
            }
            dtypes = dtypes->next;
            parameter = parameter->sibling;
            num_parameters++; num_expected++;
        }
        while (parameter != NULL) {
            num_parameters += 1;
            parameter = parameter->sibling;
        }
        while (dtypes != NULL) {
            num_expected += 1;
            dtypes = dtypes->next;
        }
        if (num_parameters != num_expected) {
            printf("Line %d: Expected %d parameter, has %d.\n", node->line_number, num_expected, num_parameters);
            *errors += 1;
            callErrors += 1;
        }
        if (callErrors > 0){
            printf("\tFunction signature: (");
            dtypes = stEntry->parameterTypes;
            int first = 1;
            while (dtypes!=NULL){
                if (!first) printf(", ");
                first = 0;
                printf("%s", printType(dtypes->type));
                dtypes = dtypes->next;
            }
            printf(") declared at function at line %d\n", stEntry->definition_line_number);
        }
        return;
    }
    switch (node->kind){
        case FUNCTION_DEFINITION_NODE:
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
            if (node->type != VOID_TYPE && (stEntry == NULL || stEntry->scope_level < scope_level)){
                insertVariable(symbolicTable, node->name, node->type, node->line_number, scope_level);
                addDeleteStack(stack, node->name);
            }
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
        case ASSIGNMENT_NODE:
            node->type = VOID_TYPE;
            break;
        case EXPRESSION_NODE:
            if (debug) printf("Expression: TBD\n");
            node->type = INTEGER_TYPE;
            break;
        case SUM_NODE:
            if (debug) printf("Sum\n");
            node->type = INTEGER_TYPE;
            break;
        case MULTIPLICATION_NODE:
            if (debug) printf("Multiplication\n");
            node->type = INTEGER_TYPE;
            break;
        case CONSTANT_NODE:
            if (debug) printf("Constant %s\n", node->name);
            node->type = INTEGER_TYPE;
            return;
        case CALL_NODE:
            break;
        case VAR_REFERENCE_NODE:
            if (debug) printf("Variable %s\n", node->name);
            stEntry = getSymbolicTableEntry(symbolicTable, node->name);
            if (stEntry == NULL){
                printf("Line %d: Name \'%s\' is not defined.\n", node->line_number, node->name);
                *errors += 1;
            }
            node->type = stEntry->type;
            break;
    }
    ASTNode *aux = node->firstChild;
    while (aux != NULL){
        semanticAnalysisNode(aux, symbolicTable, debug, errors, scope_level, stack);
        aux = aux->sibling;
    }
    return;
}
