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
