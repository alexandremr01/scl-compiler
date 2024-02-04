#include "symbolic_table.h"
#include "ast.h"
#include <strings.h>
#include <stdio.h>

void insertVariable(SymbolicTable *table, char *name, DataType type){
    SymbolicTableEntry *entry = newSymbolicTableEntry(name, type);
    HASH_ADD_KEYPTR(hh, table->entries, entry->name, strlen(entry->name), entry);
}

SymbolicTableEntry * newSymbolicTableEntry(char *name, DataType type){
    SymbolicTableEntry *entry = (SymbolicTableEntry *)malloc(sizeof(SymbolicTableEntry));
    entry->name = name;
    entry->type = type;
    return entry;
}

SymbolicTable* newSymbolicTable(){
    SymbolicTable *table = (SymbolicTable *)malloc(sizeof(SymbolicTable));
    table->entries = NULL;
    return table;
}

SymbolicTableEntry * getVariable(SymbolicTable *table, char *name) {
    SymbolicTableEntry *entry = NULL;
    HASH_FIND_STR(table->entries, name, entry);
    return entry;
}

void printSymbolicTable(SymbolicTable *table){
    SymbolicTableEntry *entry;

    for (entry = table->entries; entry != NULL; entry = entry->hh.next) {
        printf("name %s: type %d\n", entry->name, entry->type);
    }
}