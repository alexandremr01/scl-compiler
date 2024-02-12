#ifndef SYMBOLIC_TABLE_H_
#define SYMBOLIC_TABLE_H_

#include "external/uthash.h"

#include "datatypes.h"

#include <strings.h>
#include <stdio.h>

typedef enum symbtabEntryKind {
    FUNCTION_ENTRY, VARIABLE_ENTRY
} SymbolicTableEntryKind;

typedef struct dataTypeList {
    DataType type;
    struct dataTypeList *next;
} DataTypeList;

typedef struct symbtabEntry {
    char *name;
    DataType type;
    SymbolicTableEntryKind kind;
    int definition_line_number;
    int scope_level;
    int address;
    int hasReturn;

    DataTypeList *parameterTypes;
    struct symbtabEntry *next;
    struct symbtabEntry *locals;
    int isParameter;

    UT_hash_handle hh;         
} SymbolicTableEntry;

typedef struct symbtab {
    SymbolicTableEntry *entries;
} SymbolicTable;

void insertVariable(SymbolicTable *table, char *name, DataType type, int line_number, int scope_level);
void insertFunction(SymbolicTable *table, char *name, DataType type, int line_number, int scope_level, DataTypeList *list);

SymbolicTableEntry * getSymbolicTableEntry(SymbolicTable *table, char *name);
void removeSymbolicTableEntry(SymbolicTable *table, char *name);
SymbolicTable * newSymbolicTable();
void freeSymbolicTable(SymbolicTable *table);
void printSymbolicTable(SymbolicTable *table);

#endif