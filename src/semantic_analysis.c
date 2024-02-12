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

void freeStack(DeleteStack* curr) {
    DeleteStack *next = NULL;
    while(curr != NULL){
        next = curr->next;
        free(curr);
        curr = next;
    }
}

void semanticAnalysisNode(AbstractSyntaxTree *tree, ASTNode *node, SymbolicTable* symbolicTable, int debug, int *errors, int scope_level, DeleteStack *stack, SymbolicTableEntry *fn);

int semanticAnalysis(AbstractSyntaxTree *tree, SymbolicTable* symbolicTable, int debug){
    int errors = 0;

    DeleteStack *stack = newDeleteStack();
    semanticAnalysisNode(tree, tree->root, symbolicTable, debug, &errors, 0, stack, NULL);
    freeStack(stack);

    SymbolicTableEntry *mainFunction = getSymbolicTableEntry(symbolicTable, "main");
    if (mainFunction == NULL || mainFunction->kind != FUNCTION_ENTRY) {
        printf("Error: No function named main.\n");
        errors++;
    }
    tree->root->stEntry = mainFunction;
    return errors;
}


void semanticAnalysisNode(AbstractSyntaxTree *tree, ASTNode *node, SymbolicTable* symbolicTable, int debug, int *errors, int scope_level, DeleteStack *stack, SymbolicTableEntry *fn){
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
        stEntry = getSymbolicTableEntry(symbolicTable, node->name);
        node->stEntry = stEntry;
        scope_level++;
        while (parameters != NULL){
            semanticAnalysisNode(tree, parameters, symbolicTable, debug, errors, scope_level, stack, stEntry);
            parameters->stEntry->isParameter = 1;
            // parameters->stEntry->locals = node->stEntry->locals;
            // node->stEntry->locals = parameters->stEntry;
            parameters = parameters->sibling;
        }
        // printf("registered locals: ");
        // SymbolicTableEntry *local = node->stEntry->locals;
        // while(local != NULL) {
        //     printf("%s ", local->name);
        //     local = local->locals;
        // }
        // printf("\n");

        semanticAnalysisNode(tree, node->firstChild, symbolicTable, debug, errors, scope_level, stack, stEntry);
        if (!stEntry->hasReturn && node->type != VOID_TYPE) {
            printf("Line %d: Reached end of non-void function \'%s\' without a return value.\n", node->line_number, node->name);
            *errors += 1;
        }
        scope_level--; 
        DeleteStack *aux = stack;
        while (aux->next != NULL){
            removeSymbolicTableEntry(symbolicTable, aux->next->name);
            aux = aux->next;
        }
        freeStack(stack);
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
        node->stEntry = stEntry;
        node->type = stEntry->type;
        node->tempRegResult = -2;
        ASTNode *parameter = node->firstChild;
        DataTypeList *dtypes = stEntry->parameterTypes;

        int callErrors = 0, num_parameters=0, num_expected=0;
        while (parameter != NULL && dtypes != NULL){
            semanticAnalysisNode(tree, parameter, symbolicTable, debug, errors, scope_level, stack, fn);
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
    ASTNode *aux = node->firstChild;
    while (aux != NULL){
        semanticAnalysisNode(tree, aux, symbolicTable, debug, errors, scope_level, stack, fn);
        aux = aux->sibling;
    }
    switch (node->kind){
        case DECLARATION_NODE:
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
                node->stEntry = getSymbolicTableEntry(symbolicTable, node->name);
                if (fn != NULL){
                    node->stEntry->locals = fn->locals;
                    fn->locals = node->stEntry;
                }
            }
            
            break;
        case RETURN_NODE: 
            if (node->firstChild->type != fn->type){
                printf("Line %d: function of type %s returning type %s.\n", node->line_number, printType(fn->type), printType(node->firstChild->type));                                                                  
                *errors += 1;
            }
            fn->hasReturn = 1;
            break;
        case ASSIGNMENT_NODE:
            node->type = VOID_TYPE;
            break;
        case EXPRESSION_NODE:
            node->type = INTEGER_TYPE;
            break;
        case SUM_NODE:
            node->type = INTEGER_TYPE;
            break;
        case SUBTRACTION_NODE:
            node->type = INTEGER_TYPE;
            break;
        case MULTIPLICATION_NODE:
            node->type = INTEGER_TYPE;
            break;
        case CONSTANT_NODE:
            node->type = INTEGER_TYPE;
            return;
        case VAR_REFERENCE_NODE:
            stEntry = getSymbolicTableEntry(symbolicTable, node->name);
            if (stEntry == NULL){
                printf("Line %d: Name \'%s\' is not defined.\n", node->line_number, node->name);
                *errors += 1;
                node->type = VOID_TYPE;
            } else {
                node->type = stEntry->type;
                node->stEntry = stEntry;
            }         
            break;
    }
    return;
}
