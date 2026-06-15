#ifndef mavix_value_h
#define mavix_value_h

#include "common.h"


typedef enum {
  VAL_BOOL,
  VAL_NIL,
  VAL_NUMBER,
} ValueType;

// 16 byte value
typedef struct {
  ValueType type;
  union {
    bool boolean;
    double number;
  } as;

} Value;

// Macros for type checking and casting values
#define IS_BOOL(value)    ((value).type == VAL_BOOL)
#define IS_NIL(value)     ((value).type == VAL_NIL)
#define IS_NUMBER(value)  ((value).type == VAL_NUMBER)


// Macros for casting values to their underlying types
#define AS_BOOL(value)    ((value).as.boolean)
#define AS_NUMBER(value)  ((value).as.number)


// Macros for creating values of different types
#define BOOL_VAL(value)       ((Value) {VAL_BOOL, {.boolean = value}})
#define NIL_VAL               ((Value){VAL_NIL, {.number = 0}})
#define NUMBER_VAL(value)     ((Value){VAL_NUMBER, {.number = value}})


typedef struct {
  int capacity;
  int count;
  Value* values;
} ValueArray;


void initValueArray(ValueArray *array);
void writeValueArray(ValueArray *array, Value value);
void freeValueArray(ValueArray *array);
void printValue(Value val);

#endif