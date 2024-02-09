#include "register_mapping.h"

#define MAX_REGISTER_NAME 6

typedef struct adjacencyListElement {
    int value;
    struct adjacencyListElement *next;
} AdjacencyListElement;

typedef struct dependenciesGraph {
    AdjacencyListElement **adjList;
    int numNodes;
} DependenciesGraph;

void printGraph(DependenciesGraph *dg){
    for (int node=0; node<dg->numNodes; node++){
        printf("Neighbors of %d:", node);
        AdjacencyListElement *el = dg->adjList[node];
        while(el != NULL){
            printf("%d ", el->value);
            el = el->next;
        }
        printf("\n");
    }
}

void addEdge(DependenciesGraph *dg, int src, int dest) {
    AdjacencyListElement *firstEl = dg->adjList[src]->next;
    AdjacencyListElement *newEl = (AdjacencyListElement *) malloc(sizeof(AdjacencyListElement));
    newEl->next = firstEl;
    newEl->value = dest;
    dg->adjList[src]->next = newEl;
}

DependenciesGraph *buildDependencyGraph(IntermediateRepresentation *ir) {
    int number_temporaries = ir->nextTempReg;

    DependenciesGraph *dg = (DependenciesGraph *) malloc(sizeof(DependenciesGraph));
    dg->adjList = (AdjacencyListElement **) malloc(number_temporaries*sizeof(AdjacencyListElement*));
    dg->numNodes = number_temporaries;

    for (int i=0; i<number_temporaries; i++){
        // all of them will start with a head element
        dg->adjList[i] = (AdjacencyListElement *) malloc(sizeof(AdjacencyListElement));
        dg->adjList[i]->value = -1;
    }

    int *lifeStart = (int *) malloc(number_temporaries*sizeof(int)); 
    int *lifeEnd = (int *) malloc(number_temporaries*sizeof(int)); 
    for (int i=0; i<number_temporaries; i++){
        lifeStart[i] = -1;
        lifeEnd[i] = -1;
    }


    IRNode *irNode = ir->head;
    int instruction = 0;
    while (irNode != NULL){
        if (irNode->instruction != COMMENT) {
            if (lifeStart[irNode->dest] == -1) 
                lifeStart[irNode->dest] = instruction;
            lifeEnd[irNode->dest] = instruction;
            if (irNode->sourceKind == REG_SOURCE)
                lifeEnd[irNode->source] = instruction;
        }
        irNode = irNode->next;
        instruction++;
    }

    for (int i=0; i<number_temporaries; i++){
        printf("Register #%d: %d - %d\n", i, lifeStart[i], lifeEnd[i]);
    }

    for (int i=0; i<number_temporaries; i++){
        for (int j=i+1; j<number_temporaries; j++){
            if (lifeStart[i] <= lifeEnd[j] && lifeStart[j] <= lifeEnd[i]) {
                addEdge(dg, i, j);
                addEdge(dg, j, i);
            }
        }
    }

    return dg;
}

RegisterMapping *newRegisterMapping(IntermediateRepresentation *ir){
    RegisterMapping *rm = (RegisterMapping *) malloc(sizeof(RegisterMapping));
    int number_temporaries = ir->nextTempReg;
    rm->map = (char **) malloc(number_temporaries*sizeof(char*));

    for (int i=0; i<number_temporaries; i++){
        rm->map[i] = (char *) malloc(MAX_REGISTER_NAME*sizeof(char));
        strcpy(rm->map[i], "rx0");
    }

    DependenciesGraph *dg = buildDependencyGraph(ir);
    printGraph(dg);

    return rm;
}

char *getRegister(RegisterMapping *rm, int temporary){
    return rm->map[temporary];
}

