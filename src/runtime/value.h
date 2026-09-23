#ifndef MELLOW_VALUE_H
#define MELLOW_VALUE_H

#include <stddef.h>

typedef enum {
    VALUE_NULL,
    VALUE_NUMBER,
    VALUE_STRING,
    VALUE_BOOL,
    VALUE_ARRAY,
    VALUE_LIST
} ValueType;

typedef struct Value Value;
typedef struct {
    Value *items;
    size_t count;
    size_t capacity;
} Collection;

struct Value {
    ValueType type;
    double number;
    int boolean;
    char *string;
    Collection *collection;
};

Value null_value(void);
Value number_value(double number);
Value bool_value(int boolean);
Value string_value(const char *text);
Value collection_value(ValueType type);
Value copy_value(Value value);
void free_value(Value *value);
int value_equal(Value left, Value right);
const char *value_type_name(Value value);
void print_value(Value value);
void collection_append(Value value, Value item);

#endif