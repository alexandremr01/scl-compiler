#include "objectcode.h"

#include <string.h>
#include <stdlib.h>

ObjectCode *newObjectCode(){
    ObjectCode *obj = (ObjectCode *) malloc(sizeof(ObjectCode));
    obj->next = NULL;
    obj->include = 1;
    obj->binary = 0;
    strcpy(obj->assembly, "");

    return obj;
}

void freeObjectCode(ObjectCode *objCode){
    ObjectCode *currObj = objCode;
    while (currObj != NULL){
        objCode = objCode->next;
        free(currObj);
        currObj = objCode;
    }
}

void writeAssembly(ObjectCode *objCode, FILE *f) {
    ObjectCode *currObj = objCode;
    while (currObj != NULL){
        if (currObj->include && strlen(currObj->assembly) > 0) 
            fprintf(f, "%s\n", currObj->assembly);
        currObj = currObj->next;
    }
}

void writeBinary(ObjectCode *objCode, FILE *f) {
    ObjectCode *currObj = objCode;
    while (currObj != NULL){
        if (currObj->include) 
            fwrite(&currObj->binary, sizeof(currObj->binary), 1, f);
        currObj = currObj->next;
    }
}