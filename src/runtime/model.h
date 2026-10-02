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

typedef enum {
    RETURN_UNSPECIFIED,
    RETURN_STRING,
    RETURN_NUMBER,
    RETURN_BOOLEAN,
    RETURN_NOTHING
} FunctionReturnType;

typedef struct Function {
    char *name;
    char **parameters;
    size_t parameter_count;
    size_t body_start;
    size_t body_end;
    TokenList *token_source;
    struct Class *owner;
    FunctionReturnType return_type;
    int is_private;
    int is_static;
    struct Function *next;
} Function;

typedef struct Field {
    char *name;
    Value value;
    struct Class *owner;
    struct Field *next;
    int is_private;
    int is_static;
} Field;

typedef struct Class {
    char *name;
    struct Class *parent;
    Field *fields;
    Field *static_fields;
    Function *methods;
    TokenList *token_source;
    struct Class *next;
    int is_private;
} Class;

typedef struct Instance {
    Class *class_info;
    Field *fields;
} Instance;

typedef enum {
    ERROR_VALUE,
    ERROR_RECURSION,
    ERROR_DIVISION,
    ERROR_SYNTAX
} ErrorType;

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
    ErrorType error_type;
    int exception_raised;
    size_t call_depth;
    int has_main;
    int break_signal;
    int continue_signal;
    int return_signal;
    Value return_value;
    TokenList **modules;
    size_t module_count;
    size_t module_capacity;
} Runtime;

#endif