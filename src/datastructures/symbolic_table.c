#include "symbolic_table.h"

SymbolicTableEntry * newSymbolicTableEntry(char *name, DataType type, SymbolicTableEntryKind kind, int line_number, int scope_level){
    SymbolicTableEntry *entry = (SymbolicTableEntry *)malloc(sizeof(SymbolicTableEntry));
    entry->name = name;
    entry->type = type;
    entry->kind = kind;
    entry->definition_line_number = line_number;
    entry->scope_level = scope_level;
    entry->address = -1;
    entry->next = NULL;
    entry->hasReturn = 0;
    entry->locals = NULL;
    entry->isParameter = 0;
    entry->hasBody = 0;
    entry->size = 0;
    entry->offset = 0;
    return entry;
}


SymbolicTableEntry *insertSymbolicTable(SymbolicTable *table, char *name, SymbolicTableEntryKind kind, DataType type, int line_number, int scope_level){
    SymbolicTableEntry *newEntry = newSymbolicTableEntry(name, type, kind, line_number, scope_level);
    newEntry->type = type;
    SymbolicTableEntry *entry = NULL;
    HASH_FIND_STR(table->entries, name, entry);
    if (entry == NULL){
        HASH_ADD_KEYPTR(hh, table->entries, newEntry->name, strlen(newEntry->name), newEntry);
    } else {
        newEntry->next = entry;
        SymbolicTableEntry *aux = NULL;
        HASH_REPLACE_STR(table->entries, name, newEntry, aux);
    }
    return newEntry;
}

void insertVariable(SymbolicTable *table, char *name, DataType type, int line_number, int scope_level){
    insertSymbolicTable(table, name, VARIABLE_ENTRY, type, line_number, scope_level);
}

void insertFunction(SymbolicTable *table, char *name, DataType type, int line_number, int scope_level, DataTypeList *list){
    SymbolicTableEntry *newEntry = insertSymbolicTable(table, name, FUNCTION_ENTRY, type, line_number, scope_level);
    newEntry->parameterTypes = list;
}


SymbolicTable* newSymbolicTable(){
    SymbolicTable *table = (SymbolicTable *)malloc(sizeof(SymbolicTable));
    table->entries = NULL;
    return table;
}

void freeSymbolicTable(SymbolicTable *table) {
    SymbolicTableEntry *entry;
    for (entry = table->entries; entry != NULL; entry = entry->hh.next) {
        HASH_DELETE(hh, table->entries, entry);
    }
    free(table);
}

SymbolicTableEntry * getSymbolicTableEntry(SymbolicTable *table, char *name) {
    SymbolicTableEntry *entry = NULL;
    HASH_FIND_STR(table->entries, name, entry);
    return entry;
}

void removeSymbolicTableEntry(SymbolicTable *table, char *name) {
    SymbolicTableEntry *entry = getSymbolicTableEntry(table, name);
    SymbolicTableEntry *aux = NULL;
    if (entry->next == NULL)
        HASH_DELETE(hh, table->entries, entry);
    else HASH_REPLACE_STR(table->entries, name, entry->next, aux);
}

void printSymbolicTable(SymbolicTable *table){
    SymbolicTableEntry *entry;

    for (entry = table->entries; entry != NULL; entry = entry->hh.next) {
        printf("name %s: type %d\n", entry->name, entry->type);
    }
}