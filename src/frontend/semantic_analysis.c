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

void semanticAnalysisNode(ASTNode *node, SymbolicTable* symbolicTable, int *errors, int scope_level, DeleteStack *stack, SymbolicTableEntry *fn);

int semanticAnalysis(AbstractSyntaxTree *tree, SymbolicTable* symbolicTable){
    int errors = 0;

    DeleteStack *stack = newDeleteStack();
    semanticAnalysisNode(tree->root, symbolicTable, &errors, 0, stack, NULL);
    freeStack(stack);

    SymbolicTableEntry *mainFunction = getSymbolicTableEntry(symbolicTable, "main");
    if (mainFunction == NULL || mainFunction->kind != FUNCTION_ENTRY) {
        printf("Error: No function named main.\n");
        errors++;
    }
    tree->root->stEntry = mainFunction;
    return errors;
}


void semanticAnalysisNode(ASTNode *node, SymbolicTable* symbolicTable, int *errors, int scope_level, DeleteStack *stack, SymbolicTableEntry *fn){
    if (node == NULL) return;
    SymbolicTableEntry * stEntry;

    if (node->kind == FUNCTION_DEFINITION_NODE || node->kind == FUNCTION_DECLARATION_NODE) {        
        // iterate over parameters and add them to datatypelist
        ASTNode *p = node->firstChild; 
        DataTypeList *head=NULL, *list = NULL;
        while(p != NULL && p->kind==DECLARATION_NODE){
            if (list == NULL){
                list = (DataTypeList *)malloc(sizeof(DataTypeList));
                list->next=NULL;
                head = list;
            } else {
                list->next = (DataTypeList *)malloc(sizeof(DataTypeList));
                list = list->next;
                list->next = NULL;
            }
            list->type = p->type;
            p = p->sibling;
        }

        stEntry = getSymbolicTableEntry(symbolicTable, node->name);
        int alreadyDeclared = stEntry != NULL && stEntry->scope_level == scope_level;
        int hasBody = p != NULL;
        int alreadyHadBody = stEntry != NULL && stEntry->hasBody;

        if ((alreadyDeclared && !hasBody) || (alreadyHadBody && hasBody)){
            printf("Line %d: Name \'%s\' already in use. First defined in line %d.\n", node->line_number, node->name, stEntry->definition_line_number);
            *errors += 1;
        } 

        if (hasBody && alreadyDeclared && !alreadyHadBody) {
            printf("TODO: validate matching \n");
        }

        if (!alreadyDeclared) {
            insertFunction(
                symbolicTable, 
                node->name, 
                node->type, 
                node->line_number, 
                scope_level,
                head
            );
        } else {
            // DataTypeList *dtListDeclaration = head;
            // DataTypeList *dtListPrev = stEntry->;
            // while (parameter != NULL && auxDTList != NULL){
            //     if (dtypes->type != parameter->type) {
            //         printf("Line %d: Parameter #%d of incorrect type. Has type %s, expected %s.\n", parameter->line_number, num_parameters, printType(parameter->type), printType(dtypes->type));
            //         *errors += 1;
            //         callErrors += 1;
            //     }
            //     dtypes = dtypes->next;
            //     parameter = parameter->sibling;
            //     num_parameters++; num_expected++;
            // }
            freeDTList(head);
        }
        
        addDeleteStack(stack, node->name);
    
        
        stEntry = getSymbolicTableEntry(symbolicTable, node->name);
        node->stEntry = stEntry;
        scope_level++;
    
        // printf("registered locals: ");
        // SymbolicTableEntry *local = node->stEntry->locals;
        // while(local != NULL) {
        //     printf("%s ", local->name);
        //     local = local->locals;
        // }
        // printf("\n");
        if (p!=NULL) {
            stack = newDeleteStack();
            p = node->firstChild; 
            while (p!= NULL && p->kind == DECLARATION_NODE){
                semanticAnalysisNode(p, symbolicTable, errors, scope_level, stack, stEntry);
                p->stEntry->isParameter = 1;
                p = p->sibling;
            }
            
            stEntry->hasBody = 1;
            semanticAnalysisNode(p, symbolicTable, errors, scope_level, stack, stEntry);
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
        }
        
        return;
    }

    if (node->kind == CALL_NODE) {
        stEntry = getSymbolicTableEntry(symbolicTable, node->name);
        if (stEntry == NULL){
            printf("Line %d: Name \'%s\' does not exist.\n", node->line_number,node->name);
            *errors += 1;
            return;
        } else if (stEntry->kind != FUNCTION_ENTRY){
            printf("Line %d: Trying to call non-callable entity \'%s\'.\n", node->line_number, node->name);
            *errors += 1;
            return;
        }

        node->stEntry = stEntry;
        node->type = stEntry->type;
        node->tempRegResult = -2;
        ASTNode *parameter = node->firstChild;
        DataTypeList *dtypes = stEntry->parameterTypes;

        int callErrors = 0, num_parameters=0, num_expected=0;
        while (parameter != NULL && dtypes != NULL){
            semanticAnalysisNode(parameter, symbolicTable, errors, scope_level, stack, fn);
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
        semanticAnalysisNode(aux, symbolicTable, errors, scope_level, stack, fn);
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
            int arraySize = 1;
            if (node->firstChild != NULL && node->firstChild->kind == VAR_INDEXING_NODE) {
                arraySize = atoi(node->firstChild->name);
            }
            if (node->type != VOID_TYPE && (stEntry == NULL || stEntry->scope_level < scope_level)){
                insertVariable(symbolicTable, node->name, node->type, node->line_number, scope_level, arraySize);
                addDeleteStack(stack, node->name);
                node->stEntry = getSymbolicTableEntry(symbolicTable, node->name);
                if (fn != NULL){
                    node->stEntry->locals = fn->locals;
                    fn->locals = node->stEntry;
                }
            }
            
            break;
        case RETURN_NODE: 
            if (fn->type == VOID_TYPE && node->firstChild != NULL ) {
                printf("Line %d: function of type void returning type %s.\n", node->line_number, printType(node->firstChild->type));                                                                  
                *errors += 1;
            } else if (node->firstChild != NULL && node->firstChild->type != fn->type){
                printf("Line %d: function of type %s returning type %s.\n", node->line_number, printType(fn->type), printType(node->firstChild->type));                                                                  
                *errors += 1;
            }
            fn->hasReturn = 1;
            break;
        case ASSIGNMENT_NODE:
             if (node->firstChild->stEntry != NULL && node->firstChild->stEntry->kind != VARIABLE_ENTRY) {
                printf("Line %d: incorrect assignment to non-assignable entity \'%s\'.\n", node->line_number, node->firstChild->stEntry->name);                                                                  
                *errors += 1;
            } else if (
                node->firstChild->type != NONE_TYPE &&
                node->firstChild->sibling->type != NONE_TYPE &&
                node->firstChild->type != node->firstChild->sibling->type
            ) {
                // if one of the types is none, it is ok (e.g. undeclared variable)
                if (canImplicitCast(node->firstChild->sibling->type, node->firstChild->type)){
                    node->firstChild->sibling->type = node->firstChild->type;
                } else {
                    printf("Line %d: incorrect assignment from type %s to variable of type %s.\n", node->line_number, printType(node->firstChild->sibling->type), printType(node->firstChild->type));                                                                  
                    *errors += 1;
                }
            }
            node->type = VOID_TYPE;
            break;
        case EXPRESSION_NODE:
            node->type = node->firstChild->type;
            break;
        case SUM_NODE:
        case SUBTRACTION_NODE:
        case MULTIPLICATION_NODE:
            if (node->firstChild->type != node->firstChild->sibling->type) {
                DataType priorityType = implicitCastPriority(
                    node->firstChild->type,
                    node->firstChild->sibling->type
                );
                if (canImplicitCast(node->firstChild->type, priorityType) &&
                    canImplicitCast(node->firstChild->sibling->type, priorityType)) {
                    node->type = priorityType;
                    node->firstChild->type = priorityType;
                    node->firstChild->sibling->type = priorityType;
                } else {
                    printf("Line %d: Cannot cast %s and %s to the same type.\n", 
                        node->line_number,
                        printType(node->firstChild->type),
                        printType(node->firstChild->sibling->type)
                    );
                    *errors += 1;
                }
            } else node->type = node->firstChild->type;
            break;
        case LT_NODE:
        case GT_NODE:
        case LEQ_NODE:
        case GEQ_NODE:
        case EQ_NODE:
        case DIFF_NODE:
        case CONSTANT_NODE:
            node->type = INTEGER_TYPE;
            if (node->name != NULL && strchr(node->name, '.') != NULL)
                node->type = FLOAT_TYPE;
            return;
        case VAR_REFERENCE_NODE:
            stEntry = getSymbolicTableEntry(symbolicTable, node->name);

            if (stEntry == NULL){
                printf("Line %d: Name \'%s\' is not defined.\n", node->line_number, node->name);
                *errors += 1;
                node->type = NONE_TYPE;
            } else if (node->firstChild != NULL && node->firstChild->type != INTEGER_TYPE){
                printf("Line %d: Index expression must be of integer type.\n", node->line_number);
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
