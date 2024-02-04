#include "symbolic_table.h"
#include "ast.h"
#include <strings.h>
#include <stdio.h>

SymbolicTableEntry * newSymbolicTableEntry(char *name, DataType type, SymbolicTableEntryKind kind){
    SymbolicTableEntry *entry = (SymbolicTableEntry *)malloc(sizeof(SymbolicTableEntry));
    entry->name = name;
    entry->type = type;
    entry->kind = kind;
    return entry;
}

void insertVariable(SymbolicTable *table, char *name, DataType type){
    SymbolicTableEntry *entry = newSymbolicTableEntry(name, type, VARIABLE_ENTRY);
    HASH_ADD_KEYPTR(hh, table->entries, entry->name, strlen(entry->name), entry);
}

void insertFunction(SymbolicTable *table, char *name, DataType type){
    SymbolicTableEntry *entry = newSymbolicTableEntry(name, type, FUNCTION_ENTRY);
    HASH_ADD_KEYPTR(hh, table->entries, entry->name, strlen(entry->name), entry);
}

SymbolicTable* newSymbolicTable(){
    SymbolicTable *table = (SymbolicTable *)malloc(sizeof(SymbolicTable));
    table->entries = NULL;
    return table;
}

SymbolicTableEntry * getSymbolicTableEntry(SymbolicTable *table, char *name) {
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