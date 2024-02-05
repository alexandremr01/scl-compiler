#include "symbolic_table.h"

SymbolicTableEntry * newSymbolicTableEntry(char *name, DataType type, SymbolicTableEntryKind kind, int line_number, int scope_level){
    SymbolicTableEntry *entry = (SymbolicTableEntry *)malloc(sizeof(SymbolicTableEntry));
    entry->name = name;
    entry->type = type;
    entry->kind = kind;
    entry->definition_line_number = line_number;
    entry->scope_level = scope_level;
    return entry;
}

void insertSymbolicTable(SymbolicTable *table, char *name, SymbolicTableEntryKind kind, DataType type, int line_number, int scope_level){
    SymbolicTableEntry *newEntry = newSymbolicTableEntry(name, type, kind, line_number, scope_level);
    
    SymbolicTableEntry *entry = NULL;
    HASH_FIND_STR(table->entries, name, entry);
    if (entry == NULL){
        HASH_ADD_KEYPTR(hh, table->entries, newEntry->name, strlen(newEntry->name), newEntry);
    } else {
        newEntry->next = entry;
        SymbolicTableEntry *aux = NULL;
        HASH_REPLACE_STR(table->entries, name, newEntry, aux);
    }
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