#include <stdio.h>

#include "memory.h"
#include "value.h"

void initValueArray(ValueArray* array) {
    array->values = NULL;
    array->capacity = 0;
    array->count = 0;
}


void writeValueArray(ValueArray* array, Value value) {
    if (array->capacity < array->count + 1) {
      int oldCapacity = array->capacity;
      array->capacity = GROW_CAPACITY(oldCapacity);
      array->values = GROW_ARRAY(Value, array->values,
                                 oldCapacity, array->capacity);
    }
  
    array->values[array->count] = value;
    array->count++;
}



void freeValueArray(ValueArray* array) {
    FREE_ARRAY(Value, array->values, array->capacity);
    initValueArray(array);
}


void printValue(Value value) {
    switch (value.type) {
        case VAL_BOOL:
            printf(AS_BOOL(value) ? "true" : "false");
            break;
        case VAL_NIL: printf("nil"); break;
        case VAL_NUMBER: printf("%g", AS_NUMBER(value)); break;
    }
}


/**
 * Compares two Value objects for equality.
 *
 * @param a The first Value to compare.
 * @param b The second Value to compare.
 * @return true if the two Value objects are equal, false otherwise.
 */
bool valuesEqual(Value a, Value b) {
  if (a.type != b.type) return false;
  switch (a.type) {
    case VAL_BOOL:   return AS_BOOL(a) == AS_BOOL(b);
    case VAL_NIL:    return true;
    case VAL_NUMBER: return AS_NUMBER(a) == AS_NUMBER(b);
    default:         return false; // Unreachable.
  }
}