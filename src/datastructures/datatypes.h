#ifndef DATATYPES_H_
#define DATATYPES_H_

typedef enum dataType {
    NONE_TYPE, INTEGER_TYPE, VOID_TYPE, FLOAT_TYPE
} DataType;

const char *printType(DataType d);
int canImplicitCast(DataType from, DataType to);
int getSize(DataType d);
int implicitCastPriority(DataType a, DataType b);

#endif