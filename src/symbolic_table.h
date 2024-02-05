#ifndef SYMBOLIC_TABLE_H_
#define SYMBOLIC_TABLE_H_

#include "external/uthash.h"

#include "datatypes.h"
#include "ast.h"

#include <strings.h>
#include <stdio.h>

typedef enum symbtabEntryKind {
    FUNCTION_ENTRY, VARIABLE_ENTRY
} SymbolicTableEntryKind;

typedef struct symbtabEntry {
    char *name;
    DataType type;
    SymbolicTableEntryKind kind;
    int definition_line_number;

    UT_hash_handle hh;         
} SymbolicTableEntry;

typedef struct symbtab {
    SymbolicTableEntry *entries;
} SymbolicTable;

void insertVariable(SymbolicTable *table, char *name, DataType type, int line_number);
void insertFunction(SymbolicTable *table, char *name, DataType type, int line_number);

SymbolicTableEntry * getSymbolicTableEntry(SymbolicTable *table, char *name);
SymbolicTable * newSymbolicTable();
void printSymbolicTable(SymbolicTable *table);

#endif