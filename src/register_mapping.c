#include "register_mapping.h"

#define MAX_REGISTER_NAME 6

RegisterMapping *newRegisterMapping(IntermediateRepresentation *ir){
    RegisterMapping *rm = (RegisterMapping *) malloc(sizeof(RegisterMapping));
    int number_registers = ir->nextTempReg;
    rm->map = (char **) malloc(number_registers*sizeof(char*));

    for (int i=0; i<number_registers; i++){
        rm->map[i] = (char *) malloc(MAX_REGISTER_NAME*sizeof(char));
        strcpy(rm->map[i], "rx0");
    }
    return rm;
}

char *getRegister(RegisterMapping *rm, int temporary){
    return rm->map[temporary];
}
