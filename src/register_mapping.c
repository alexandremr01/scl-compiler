#include "register_mapping.h"

#define MAX_REGISTER_NAME 6

typedef struct adjacencyListElement {
    int value;
    struct adjacencyListElement *next;
} AdjacencyListElement;

typedef struct dependenciesGraph {
    AdjacencyListElement **adjList;
    int *num_neighbors;
    int numNodes;
} DependenciesGraph;

void printGraph(DependenciesGraph *dg){
    for (int node=0; node<dg->numNodes; node++){
        printf("Neighbors of %d: ", node);
        AdjacencyListElement *el = dg->adjList[node]->next;
        while(el != NULL){
            printf("%d ", el->value);
            el = el->next;
        }
        printf("\n");
    }
}

typedef struct intStack {
    int value;
    struct intStack *next;
} IntStack;

IntStack * pushStack(IntStack *stack, int v) {
    IntStack* el = (IntStack*) malloc(sizeof(IntStack));
    el->value = v;
    el->next = stack;
    return el;
}

void freeIntStack(IntStack* curr) {
    IntStack *next = NULL;
    while(curr != NULL){
        next = curr->next;
        free(curr);
        curr = next;
    }
}

int *colorGraph(DependenciesGraph *dg, int number_temporaries, int num_colors) {
    int *map = (int*) malloc(number_temporaries*sizeof(int));
    int *active = (int*) malloc(number_temporaries*sizeof(int));
    for (int i=0; i<number_temporaries; i++) active[i] = 1;
    IntStack *stack = NULL;
    int num_nodes = number_temporaries;
    while (num_nodes > 0) {
        int max_neighbors_under_k = 0, selected_node = -1;
        for (int i=0; i<number_temporaries; i++) {
            if (active[i] && (selected_node==-1 || dg->num_neighbors[i] > max_neighbors_under_k)) {
                max_neighbors_under_k = dg->num_neighbors[i];
                selected_node = i;
            }
        }
        stack = pushStack(stack, selected_node);
        active[selected_node] = 0;
        // remove all nodes linked to the selected
        dg->num_neighbors[selected_node] = 0;
        AdjacencyListElement *el = dg->adjList[selected_node]->next;
        while(el != NULL){
            dg->num_neighbors[el->value]--;
            el = el->next;
        }

        // AdjacencyListElement *el = dg->adjList[selected_node]->next;
        // while(el != NULL){
        //     AdjacencyListElement *el2 = dg->adjList[el->value]->next;
        //     AdjacencyListElement *aux2 = dg->adjList[el->value];
        //     while(el2 != NULL){
        //         if (el2->value == selected_node) {
        //             aux2->next = el2->next;
        //             free(el2);
        //             break;
        //         }
        //         el2 = el2->next;
        //         aux2 = aux2->next;
        //     }
        //     AdjacencyListElement *aux = el->next;
        //     free(el);
        //     el = aux;
        // }
        // dg->adjList[selected_node]->next = NULL;

        num_nodes -= 1;
    }
    printf("Stack: ");
    IntStack *stackAux = stack;
    while (stackAux!=NULL) {
        printf("%d ", stackAux->value);
        stackAux = stackAux->next;
    }
    printf("\n");
    // pop and give colors
    stackAux = stack;
    while (stackAux!=NULL) {
        // iterate colors
        //   iterate neighbors
        //     neighbor active and has color: color unavailable
        //   if color is available: Mark color
        //   activate this node
        int foundColor = 0;
        for (int color = 0; color < num_colors; color++){
            AdjacencyListElement *el = dg->adjList[stackAux->value]->next;
            int color_available = 1;
            while(el != NULL){
                color_available = color_available && !(active[el->value] && map[el->value] == color);
                el = el->next;
            }
            if (color_available) {
                map[stackAux->value] = color;
                foundColor = 1;
                break;
            }
        }
        if (!foundColor) {
            printf("Fatal: Error on register assignment\n");
            break;
        }
        active[stackAux->value] = 1;
        stackAux = stackAux->next;
    }

    printf("Color map: ");
    for (int i=0; i<number_temporaries; i++) {
        printf("%d ", map[i]);
    }
    printf("\n");

    freeIntStack(stack);
    free(active);

    return map;
}

void addEdge(DependenciesGraph *dg, int src, int dest) {
    AdjacencyListElement *firstEl = dg->adjList[src]->next;
    AdjacencyListElement *newEl = (AdjacencyListElement *) malloc(sizeof(AdjacencyListElement));
    newEl->next = firstEl;
    newEl->value = dest;
    dg->adjList[src]->next = newEl;
    dg->num_neighbors[src]+=1;
}

void deleteDependencyGraph(DependenciesGraph *dg) {
    free(dg->num_neighbors);
    for (int i=0; i<dg->numNodes; i++){
        AdjacencyListElement *el = dg->adjList[i], *next = NULL;
        while(el != NULL){
            next = el->next;
            free(el);
            el = next;
        }
    }
    free(dg->adjList);
    free(dg);
}

DependenciesGraph *buildDependencyGraph(IntermediateRepresentation *ir) {
    int number_temporaries = ir->nextTempReg;

    DependenciesGraph *dg = (DependenciesGraph *) malloc(sizeof(DependenciesGraph));
    dg->adjList = (AdjacencyListElement **) malloc(number_temporaries*sizeof(AdjacencyListElement*));
    dg->num_neighbors = (int*) malloc(number_temporaries*sizeof(int));
    dg->numNodes = number_temporaries;

    for (int i=0; i<number_temporaries; i++){
        // all of them will start with a head element
        dg->adjList[i] = (AdjacencyListElement *) malloc(sizeof(AdjacencyListElement));
        dg->adjList[i]->value = -1;
        dg->adjList[i]->next = NULL;
        dg->num_neighbors[i] = 0;
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
    
    free(lifeStart);
    free(lifeEnd);

    return dg;
}

RegisterMapping *newRegisterMapping(IntermediateRepresentation *ir){
    RegisterMapping *rm = (RegisterMapping *) malloc(sizeof(RegisterMapping));
    int number_temporaries = ir->nextTempReg;
    DependenciesGraph *dg = buildDependencyGraph(ir);
    printGraph(dg);
    rm->map = colorGraph(dg, number_temporaries, 10);
    deleteDependencyGraph(dg);

    return rm;
}

void freeRegisterMapping(RegisterMapping *rm) {
    free(rm->map);
    free(rm);
}

int getRegisterAssignment(RegisterMapping *rm, int temporary){
    return rm->map[temporary];
}

