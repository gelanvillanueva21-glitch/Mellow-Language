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
    TokenList *token_source;
    struct Class *owner;
    int is_private;
    struct Function *next;
} Function;

typedef struct Field {
    char *name;
    Value value;
    struct Field *next;
} Field;

typedef struct Class {
    char *name;
    struct Class *parent;
    Field *fields;
    Function *methods;
    struct Class *next;
} Class;

typedef struct Instance {
    Class *class_info;
    Field *fields;
} Instance;

typedef struct {
    TokenList *tokens;
    size_t current;
    Variable *variables;
    Function *functions;
    Class *classes;
    Instance *this_instance;
    Class *active_class;
    int importing_module;
    char **import_names;
    size_t import_name_count;
    int failed;
    char *error_message;
    int exception_raised;
    int break_signal;
    int continue_signal;
    int return_signal;
    Value return_value;
    TokenList **modules;
    size_t module_count;
    size_t module_capacity;
} Runtime;

#endif