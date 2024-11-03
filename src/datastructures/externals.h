#ifndef EXTERNALS_H_
#define EXTERNALS_H_

#include "../external/uthash.h"

#include "datatypes.h"

#include <strings.h>
#include <stdio.h>

typedef struct external {
    char *name;
    FILE *file;         
} External;

typedef struct externalNode {
    External *external;
    struct externalNode *next;         
} ExternalNode;

typedef struct externals {
    ExternalNode *first;
    ExternalNode *last;         
} Externals;

Externals* newExternals();
void addExternal(Externals *ext, char *name, FILE *f);
void freeExternals(Externals *ext);

#endif