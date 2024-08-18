#include "datatypes.h"

const char *printType(DataType d){
    switch(d){
        case NONE_TYPE:
            return "void";
        case INTEGER_TYPE:
            return "int";
        case VOID_TYPE:
            return "void";
        case FLOAT_TYPE:
            return "float";
    }
    return "void";
}

int canImplicitCast(DataType from, DataType to) {
    // only allow certain implicit casts - all others are forbidden
    if (from == INTEGER_TYPE && to == FLOAT_TYPE){
        return 1;
    }
    return 0;
}

int getSize(DataType d){
    switch(d){
        case NONE_TYPE:
            return 0;
        case INTEGER_TYPE:
            return 4;
        case VOID_TYPE:
            return 0;
        case FLOAT_TYPE:
            return 4;
    }
    return 0;
}
