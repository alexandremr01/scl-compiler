#include "linker.h"

// assert that all function calls have a destination
int link(IntermediateRepresentation *ir, SymbolicTable *table) {
    // give address to globals
    SymbolicTableEntry *entry;
    for (entry = table->entries; entry != NULL; entry = entry->hh.next) {
        if (entry->scope_level == 0) {
            entry->address = ir->lastAddress;
            ir->lastAddress += getSize(entry->type);
        }
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