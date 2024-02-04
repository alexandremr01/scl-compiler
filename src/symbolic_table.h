#ifndef SYMBOLIC_TABLE_H_
#define SYMBOLIC_TABLE_H_

#include "external/uthash.h"
#include "datatypes.h"

typedef struct symbtabEntry {
    char *name;
    DataType type;
    char *scope;
    UT_hash_handle hh;         
} SymbolicTableEntry;

typedef struct symbtab {
    SymbolicTableEntry *entries;
} SymbolicTable;

void insertVariable(SymbolicTable *table, char *name, DataType type);
SymbolicTableEntry * newSymbolicTableEntry(char *name, DataType type);
SymbolicTableEntry * getVariable(SymbolicTable *table, char *name);
SymbolicTable * newSymbolicTable();
void printSymbolicTable(SymbolicTable *table);

#endif