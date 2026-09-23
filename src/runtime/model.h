#ifndef MELLOW_RUNTIME_MODEL_H
#define MELLOW_RUNTIME_MODEL_H

#include "../lexer.h"
#include "value.h"

typedef struct Variable {
    char *name;
    Value value;
    int constant;
    struct Variable *next;
} Variable;

typedef struct Function {
    char *name;
    char **parameters;
    size_t parameter_count;
    size_t body_start;
    size_t body_end;
    struct Function *next;
} Function;

typedef struct {
    TokenList *tokens;
    size_t current;
    Variable *variables;
    Function *functions;
    int failed;
    int break_signal;
    int continue_signal;
    int return_signal;
    Value return_value;
} Runtime;

#endif