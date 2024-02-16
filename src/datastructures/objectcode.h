#ifndef OBJECTCODE_H_
#define OBJECTCODE_H_

#include <stdio.h>

typedef struct objectCode {
    char assembly[25];
    int include;
    int binary;

    struct objectCode *next;
} ObjectCode; 

void freeObjectCode(ObjectCode *objCode);
void writeAssembly(ObjectCode *objCode, FILE *f);
void writeBinary(ObjectCode *objCode, FILE *f);
ObjectCode *newObjectCode();

#endif