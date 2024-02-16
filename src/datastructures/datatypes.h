#ifndef DATATYPES_H_
#define DATATYPES_H_

typedef enum dataType {
    NONE_TYPE, INTEGER_TYPE, VOID_TYPE, FLOAT_TYPE
} DataType;

const char *printType(DataType d);
int getSize(DataType d);

#endif