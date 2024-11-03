#include "externals.h"

Externals *newExternals(){
    Externals *ext = (Externals *) malloc(sizeof(Externals));
    ext->first = NULL;
    ext->last = NULL;
    return ext;
}

void addExternal(Externals *ext, char *name, FILE *f) {
    ExternalNode *extNode = (ExternalNode *) malloc(sizeof(ExternalNode));
    extNode->next = NULL;
    extNode->external = (External *) malloc(sizeof(External));
    extNode->external->name = name;
    extNode->external->file = f;

    if (ext->last != NULL) {
        ext->last->next = extNode;
    } 
    ext->last = extNode;
    if (ext->first == NULL) ext->first = extNode;

}


void freeExternals(Externals *ext) {
    ExternalNode *extNode = ext->first;
    ExternalNode *next = NULL;
    while(extNode != NULL){
        next = extNode->next;
        free(extNode->external);
        free(extNode);
        extNode = next;
    }

    free(ext);
}
