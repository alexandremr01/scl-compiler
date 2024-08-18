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
    return (from == to) || (from == INTEGER_TYPE && to == FLOAT_TYPE);
}

int implicitCastPriority(DataType a, DataType b) {
    if ((a == INTEGER_TYPE && b == FLOAT_TYPE) || (b == INTEGER_TYPE && a == FLOAT_TYPE)) {
        return FLOAT_TYPE;
    }
    if (a == b) {
        return a;
    }
    return -1;
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
