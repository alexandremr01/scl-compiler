#ifndef SYMBOLIC_TABLE_H_
#define SYMBOLIC_TABLE_H_

#include "external/uthash.h"
#include "datatypes.h"

typedef enum symbtabEntryKind {
    FUNCTION_ENTRY, VARIABLE_ENTRY
} SymbolicTableEntryKind;

typedef struct symbtabEntry {
    char *name;
    DataType type;
    SymbolicTableEntryKind kind;

    UT_hash_handle hh;         
} SymbolicTableEntry;

typedef struct symbtab {
    SymbolicTableEntry *entries;
} SymbolicTable;

void insertVariable(SymbolicTable *table, char *name, DataType type);
void insertFunction(SymbolicTable *table, char *name, DataType type);

SymbolicTableEntry * getSymbolicTableEntry(SymbolicTable *table, char *name);
SymbolicTable * newSymbolicTable();
void printSymbolicTable(SymbolicTable *table);

#endif