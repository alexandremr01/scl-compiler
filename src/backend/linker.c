#include "linker.h"

// The Linker has two responsabilities:
// link with the standard library
// assert that all function calls have a destination
int link(IntermediateRepresentation *ir) {
    // give address to globals
    SymbolicTableGlobals *g = ir->globals->next;
    while (g!=NULL) {
        g->entry->address = ir->lastAddress;
        ir->lastAddress += getSize(g->entry->type);
        g = g->next;
    }

    IRNode *p = ir->head;
    // give label address to functions
    while (p != NULL) {
        if (p->instruction == LABEL){
            p->varSource->address = p->address;
        }
        p = p->next;
    }

    // identify if there is any jump to unlabeled function
    int errors = 0;
    p = ir->head;
    while (p != NULL) {
        if (p->instruction == JUMP && p->sourceKind != CONSTANT_SOURCE && p->varSource->address == -1){
            printf("Function %s not found.\n", p->varSource->name);
            errors++;
        }
        p = p->next;
    }
    return errors;
}