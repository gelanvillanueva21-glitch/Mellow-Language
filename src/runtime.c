#include "runtime.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { VALUE_NULL, VALUE_NUMBER, VALUE_STRING, VALUE_BOOL, VALUE_ARRAY, VALUE_LIST } ValueType;
typedef struct Value Value;
typedef struct { Value *items; size_t count; size_t capacity; } Collection;
struct Value { ValueType type; double number; int boolean; char *string; Collection *collection; };
typedef struct Variable { char *name; Value value; int constant; struct Variable *next; } Variable;
typedef struct Function { char *name; char **parameters; size_t parameter_count; size_t body_start; size_t body_end; struct Function *next; } Function;
typedef struct { TokenList *tokens; size_t current; Variable *variables; Function *functions; int failed; int break_signal; int continue_signal; int return_signal; Value return_value; } Runtime;
static Variable *find_variable(Runtime *runtime, const char *name);

static char *duplicate_text(const char *text) {
    size_t length = strlen(text) + 1;
    char *copy = malloc(length);
    if (copy) memcpy(copy, text, length);
    return copy;
}
static Value null_value(void) { return (Value){VALUE_NULL, 0, 0, NULL, NULL}; }
static Value number_value(double number) { return (Value){VALUE_NUMBER, number, 0, NULL, NULL}; }
static Value bool_value(int boolean) { return (Value){VALUE_BOOL, 0, boolean, NULL, NULL}; }
static Value string_value(const char *text) {
    Value value = {VALUE_STRING, 0, 0, duplicate_text(text ? text : ""), NULL};
    return value;
}
static Value interpolated_string(Runtime *runtime, const char *text) {
    char buffer[4096]; size_t used = 0;
    for (size_t i = 0; text[i] && used + 1 < sizeof(buffer); i++) {
        if (text[i] == '{') {
            size_t start = ++i;
            while (text[i] && text[i] != '}') i++;
            if (text[i] == '}') {
                char name[128]; size_t length = i - start;
                if (length >= sizeof(name)) length = sizeof(name) - 1;
                memcpy(name, text + start, length); name[length] = '\0';
                Variable *variable = find_variable(runtime, name);
                if (variable) {
                    char value[256];
                    if (variable->value.type == VALUE_STRING) snprintf(value, sizeof(value), "%s", variable->value.string);
                    else if (variable->value.type == VALUE_NUMBER) snprintf(value, sizeof(value), "%g", variable->value.number);
                    else if (variable->value.type == VALUE_BOOL) snprintf(value, sizeof(value), "%s", variable->value.boolean ? "true" : "false");
                    else snprintf(value, sizeof(value), "null");
                    size_t value_length = strlen(value);
                    if (used + value_length >= sizeof(buffer)) value_length = sizeof(buffer) - used - 1;
                    memcpy(buffer + used, value, value_length); used += value_length;
                    continue;
                }
                i = start - 1;
            }
        }
        buffer[used++] = text[i];
    }
    buffer[used] = '\0'; return string_value(buffer);
}
static Value collection_value(ValueType type) {
    Value value = {type, 0, 0, NULL, calloc(1, sizeof(Collection))};
    return value;
}
static void free_value(Value *value) {
    if (value->type == VALUE_STRING) free(value->string);
    if ((value->type == VALUE_ARRAY || value->type == VALUE_LIST) && value->collection) {
        for (size_t i = 0; i < value->collection->count; i++) free_value(&value->collection->items[i]);
        free(value->collection->items); free(value->collection);
    }
    value->string = NULL; value->collection = NULL;
}
static Value copy_value(Value value) {
    if (value.type == VALUE_STRING) return string_value(value.string);
    if (value.type != VALUE_ARRAY && value.type != VALUE_LIST) return value;
    Value copy = collection_value(value.type);
    for (size_t i = 0; i < value.collection->count; i++) {
        Collection *collection = copy.collection;
        collection->items = realloc(collection->items, (collection->count + 1) * sizeof(Value));
        collection->items[collection->count++] = copy_value(value.collection->items[i]);
    }
    return copy;
}

static Token *peek(Runtime *runtime) { return &runtime->tokens->items[runtime->current]; }
static Token *advance(Runtime *runtime) { return &runtime->tokens->items[runtime->current++]; }
static int match(Runtime *runtime, TokenType type) { if (peek(runtime)->type != type) return 0; advance(runtime); return 1; }
static void error_at(Runtime *runtime, const char *message) {
    Token *token = peek(runtime);
    fprintf(stderr, "Mellow error at %d:%d: %s\n", token->line, token->column, message);
    runtime->failed = 1;
}
static Variable *find_variable(Runtime *runtime, const char *name) {
    for (Variable *variable = runtime->variables; variable; variable = variable->next)
        if (strcmp(variable->name, name) == 0) return variable;
    return NULL;
}
static Function *find_function(Runtime *runtime, const char *name) {
    for (Function *function = runtime->functions; function; function = function->next)
        if (strcmp(function->name, name) == 0) return function;
    return NULL;
}
static void set_variable(Runtime *runtime, const char *name, Value value, int constant) {
    Variable *variable = find_variable(runtime, name);
    if (!variable) {
        variable = calloc(1, sizeof(*variable));
        variable->name = duplicate_text(name); variable->next = runtime->variables;
        runtime->variables = variable;
    } else {
        if (variable->constant) { error_at(runtime, "cannot reassign a const variable"); return; }
        free_value(&variable->value);
    }
    variable->value = copy_value(value);
    variable->constant = constant || variable->constant;
}
static void skip_lines(Runtime *runtime) { while (match(runtime, TOKEN_NEWLINE) || match(runtime, TOKEN_SEMICOLON)) {} }

static Value expression(Runtime *runtime);
static Value call_user_function(Runtime *runtime, Function *function, Value *arguments, size_t count);

static int is_number_pair(Value *arguments, size_t count) {
    return count == 2 && arguments[0].type == VALUE_NUMBER && arguments[1].type == VALUE_NUMBER;
}
static int value_equal(Value left, Value right) {
    if (left.type != right.type) return 0;
    if (left.type == VALUE_NUMBER) return left.number == right.number;
    if (left.type == VALUE_STRING) return strcmp(left.string, right.string) == 0;
    if (left.type == VALUE_BOOL) return left.boolean == right.boolean;
    if (left.type == VALUE_NULL) return 1;
    if (left.collection->count != right.collection->count) return 0;
    for (size_t i = 0; i < left.collection->count; i++)
        if (!value_equal(left.collection->items[i], right.collection->items[i])) return 0;
    return 1;
}
static const char *value_type_name(Value value) {
    static const char *names[] = {"null", "number", "string", "bool", "array", "list"};
    return names[value.type];
}
static void print_value(Value value) {
    if (value.type == VALUE_STRING) printf("%s", value.string);
    else if (value.type == VALUE_NUMBER) printf("%g", value.number);
    else if (value.type == VALUE_BOOL) printf("%s", value.boolean ? "true" : "false");
    else if (value.type == VALUE_NULL) printf("null");
    else {
        putchar('{');
        for (size_t i = 0; i < value.collection->count; i++) { if (i) printf(", "); print_value(value.collection->items[i]); }
        putchar('}');
    }
}
static void collection_append(Value value, Value item) {
    if (value.collection->count == value.collection->capacity) {
        size_t capacity = value.collection->capacity == 0 ? 4 : value.collection->capacity * 2;
        value.collection->items = realloc(value.collection->items, capacity * sizeof(Value));
        value.collection->capacity = capacity;
    }
    value.collection->items[value.collection->count++] = copy_value(item);
}

static Value call_builtin(Runtime *runtime, const char *name, Value *arguments, size_t count) {
    if (strcmp(name, "print") == 0 || strcmp(name, "Print") == 0) {
        for (size_t i = 0; i < count; i++) {
            if (i) putchar(' ');
            print_value(arguments[i]);
        }
        putchar('\n'); return null_value();
    }
    if (strcmp(name, "raise") == 0 && count == 1) {
        fprintf(stderr, "Mellow exception: "); print_value(arguments[0]); fputc('\n', stderr);
        runtime->failed = 1; return null_value();
    }
    if (count < 1 && (strcmp(name, "len") == 0 || strcmp(name, "to_string") == 0)) { error_at(runtime, "builtin needs an argument"); return null_value(); }
    if ((strcmp(name, "add") == 0 || strcmp(name, "sub") == 0 || strcmp(name, "mul") == 0 || strcmp(name, "div") == 0 || strcmp(name, "mod") == 0 || strcmp(name, "pow") == 0) && count == 2) {
        if ((strcmp(name, "add") == 0) && arguments[0].type == VALUE_STRING && arguments[1].type == VALUE_STRING) {
            size_t length = strlen(arguments[0].string) + strlen(arguments[1].string) + 1;
            char *text = malloc(length); snprintf(text, length, "%s%s", arguments[0].string, arguments[1].string);
            Value result = string_value(text); free(text); return result;
        }
        if ((strcmp(name, "add") == 0) && (arguments[0].type == VALUE_ARRAY || arguments[0].type == VALUE_LIST) && arguments[0].type == arguments[1].type) {
            Value result = copy_value(arguments[0]);
            for (size_t i = 0; i < arguments[1].collection->count; i++) collection_append(result, arguments[1].collection->items[i]);
            return result;
        }
        if (arguments[0].type != VALUE_NUMBER || arguments[1].type != VALUE_NUMBER) { error_at(runtime, "arithmetic expects numbers"); return null_value(); }
        if (strcmp(name, "add") == 0) return number_value(arguments[0].number + arguments[1].number);
        if (strcmp(name, "sub") == 0) return number_value(arguments[0].number - arguments[1].number);
        if (strcmp(name, "mul") == 0) return number_value(arguments[0].number * arguments[1].number);
        if (strcmp(name, "mod") == 0) return number_value(fmod(arguments[0].number, arguments[1].number));
        if (strcmp(name, "pow") == 0) return number_value(pow(arguments[0].number, arguments[1].number));
        if (arguments[1].number == 0) { error_at(runtime, "division by zero"); return null_value(); }
        return number_value(arguments[0].number / arguments[1].number);
    }
    if (strcmp(name, "sqrt") == 0 && count == 1 && arguments[0].type == VALUE_NUMBER) return number_value(sqrt(arguments[0].number));
    if ((strcmp(name, "inc") == 0 || strcmp(name, "dec") == 0) && count == 1 && arguments[0].type == VALUE_NUMBER) return number_value(arguments[0].number + (strcmp(name, "inc") == 0 ? 1 : -1));
    if (strcmp(name, "eq") == 0 && count == 2) return bool_value(value_equal(arguments[0], arguments[1]));
    if (strcmp(name, "neq") == 0 && count == 2) return bool_value(!value_equal(arguments[0], arguments[1]));
    if (is_number_pair(arguments, count) && (strcmp(name, "lt") == 0 || strcmp(name, "gt") == 0 || strcmp(name, "lte") == 0 || strcmp(name, "gte") == 0)) {
        if (strcmp(name, "lt") == 0) return bool_value(arguments[0].number < arguments[1].number);
        if (strcmp(name, "gt") == 0) return bool_value(arguments[0].number > arguments[1].number);
        if (strcmp(name, "lte") == 0) return bool_value(arguments[0].number <= arguments[1].number);
        return bool_value(arguments[0].number >= arguments[1].number);
    }
    if (strcmp(name, "and") == 0 && count == 2) return bool_value(arguments[0].boolean && arguments[1].boolean);
    if (strcmp(name, "or") == 0 && count == 2) return bool_value(arguments[0].boolean || arguments[1].boolean);
    if (strcmp(name, "not") == 0 && count == 1) return bool_value(!arguments[0].boolean);
    if (strcmp(name, "type") == 0 && count == 1) return string_value(value_type_name(arguments[0]));
    if (strcmp(name, "list") == 0) {
        Value result = collection_value(VALUE_LIST);
        for (size_t i = 0; i < count; i++) collection_append(result, arguments[i]);
        return result;
    }
    if (strcmp(name, "len") == 0 && arguments[0].type == VALUE_STRING) return number_value((double)strlen(arguments[0].string));
    if (strcmp(name, "len") == 0 && (arguments[0].type == VALUE_ARRAY || arguments[0].type == VALUE_LIST)) return number_value((double)arguments[0].collection->count);
    if (strcmp(name, "to_string") == 0) {
        char buffer[64];
        if (arguments[0].type == VALUE_STRING) return copy_value(arguments[0]);
        if (arguments[0].type == VALUE_NUMBER) snprintf(buffer, sizeof(buffer), "%g", arguments[0].number);
        else if (arguments[0].type == VALUE_BOOL) snprintf(buffer, sizeof(buffer), "%s", arguments[0].boolean ? "true" : "false");
        else if (arguments[0].type == VALUE_ARRAY || arguments[0].type == VALUE_LIST) { print_value(arguments[0]); return string_value("[collection]"); }
        else snprintf(buffer, sizeof(buffer), "null");
        return string_value(buffer);
    }
    fprintf(stderr, "Mellow error at %d:%d: unknown builtin '%s'\n", peek(runtime)->line, peek(runtime)->column, name);
    runtime->failed = 1; return null_value();
}

static Value expression(Runtime *runtime) {
    Token *token = advance(runtime);
    if (token->type == TOKEN_NUMBER) return number_value(strtod(token->lexeme, NULL));
    if (token->type == TOKEN_STRING || token->type == TOKEN_CHAR) return interpolated_string(runtime, token->lexeme);
    if (token->type == TOKEN_TRUE) return bool_value(1);
    if (token->type == TOKEN_FALSE) return bool_value(0);
    if (token->type == TOKEN_NULL) return null_value();
    if (token->type == TOKEN_LEFT_BRACE) {
        Value result = collection_value(VALUE_ARRAY);
        skip_lines(runtime);
        if (!match(runtime, TOKEN_RIGHT_BRACE)) {
            do {
                skip_lines(runtime);
                Value item = expression(runtime);
                collection_append(result, item);
                free_value(&item);
                skip_lines(runtime);
            } while (match(runtime, TOKEN_COMMA));
            skip_lines(runtime);
            if (!match(runtime, TOKEN_RIGHT_BRACE)) error_at(runtime, "expected } after array literal");
        }
        return result;
    }
    if (token->type == TOKEN_IDENTIFIER || token->type == TOKEN_RAISE || token->type == TOKEN_AND || token->type == TOKEN_OR || token->type == TOKEN_NOT) {
        if (match(runtime, TOKEN_LEFT_ANGLE)) {
            Value arguments[32]; size_t count = 0;
            skip_lines(runtime);
            if (strcmp(token->lexeme, "set") == 0) {
                Token *name = peek(runtime);
                if (!match(runtime, TOKEN_IDENTIFIER)) { error_at(runtime, "set expects a variable name"); return null_value(); }
                skip_lines(runtime);
                if (!match(runtime, TOKEN_COMMA)) { error_at(runtime, "set expects a value"); return null_value(); }
                Value value = expression(runtime);
                skip_lines(runtime);
                if (!match(runtime, TOKEN_RIGHT_ANGLE)) error_at(runtime, "expected > after set");
                Variable *variable = find_variable(runtime, name->lexeme);
                if (!variable) error_at(runtime, "cannot set an unknown variable");
                else if (variable->constant) error_at(runtime, "cannot set a const variable");
                else { free_value(&variable->value); variable->value = copy_value(value); }
                free_value(&value);
                return null_value();
            }
            if (!match(runtime, TOKEN_RIGHT_ANGLE)) {
                do {
                    skip_lines(runtime);
                    if (count == 32) { error_at(runtime, "too many arguments"); break; }
                    arguments[count++] = expression(runtime);
                    skip_lines(runtime);
                } while (match(runtime, TOKEN_COMMA));
                skip_lines(runtime);
                if (!match(runtime, TOKEN_RIGHT_ANGLE)) error_at(runtime, "expected > after call arguments");
            }
            Function *function = find_function(runtime, token->lexeme);
            Value result = function ? call_user_function(runtime, function, arguments, count) : call_builtin(runtime, token->lexeme, arguments, count);
            for (size_t i = 0; i < count; i++) free_value(&arguments[i]);
            return result;
        }
        Variable *variable = find_variable(runtime, token->lexeme);
        if (!variable) { error_at(runtime, "unknown variable"); return null_value(); }
        return copy_value(variable->value);
    }
    error_at(runtime, "expected an expression"); return null_value();
}

static int truthy(Value value) {
    if (value.type == VALUE_NULL) return 0;
    if (value.type == VALUE_BOOL) return value.boolean;
    if (value.type == VALUE_NUMBER) return value.number != 0;
    if (value.type == VALUE_STRING) return value.string[0] != '\0';
    return value.collection->count != 0;
}
static void skip_block(Runtime *runtime) {
    if (!match(runtime, TOKEN_LEFT_BRACKET)) { error_at(runtime, "expected [ to begin block"); return; }
    int depth = 1;
    while (peek(runtime)->type != TOKEN_EOF && depth > 0) {
        if (match(runtime, TOKEN_LEFT_BRACKET)) depth++;
        else if (match(runtime, TOKEN_RIGHT_BRACKET)) depth--;
        else advance(runtime);
    }
}
static void execute_statement(Runtime *runtime);
static void execute_block(Runtime *runtime) {
    if (!match(runtime, TOKEN_LEFT_BRACKET)) { error_at(runtime, "expected [ to begin block"); return; }
    while (peek(runtime)->type != TOKEN_EOF && peek(runtime)->type != TOKEN_RIGHT_BRACKET && !runtime->failed && !runtime->break_signal && !runtime->continue_signal && !runtime->return_signal) {
        skip_lines(runtime);
        if (peek(runtime)->type != TOKEN_RIGHT_BRACKET) execute_statement(runtime);
        skip_lines(runtime);
    }
    match(runtime, TOKEN_RIGHT_BRACKET);
}
static void declare_function(Runtime *runtime) {
    Token *name = peek(runtime);
    if (!match(runtime, TOKEN_IDENTIFIER) || !match(runtime, TOKEN_LEFT_ANGLE)) { error_at(runtime, "expected function name and parameters"); return; }
    Function *function = calloc(1, sizeof(*function));
    function->name = duplicate_text(name->lexeme);
    skip_lines(runtime);
    if (!match(runtime, TOKEN_RIGHT_ANGLE)) {
        do {
            skip_lines(runtime);
            Token *parameter = peek(runtime);
            if (!match(runtime, TOKEN_IDENTIFIER)) { error_at(runtime, "expected parameter name"); break; }
            function->parameters = realloc(function->parameters, (function->parameter_count + 1) * sizeof(char *));
            function->parameters[function->parameter_count++] = duplicate_text(parameter->lexeme);
            skip_lines(runtime);
        } while (match(runtime, TOKEN_COMMA));
        if (!match(runtime, TOKEN_RIGHT_ANGLE)) error_at(runtime, "expected > after parameters");
    }
    skip_lines(runtime);
    if (!match(runtime, TOKEN_LEFT_BRACKET)) { error_at(runtime, "expected [ after function signature"); return; }
    function->body_start = runtime->current;
    int depth = 1;
    while (peek(runtime)->type != TOKEN_EOF && depth > 0) {
        if (match(runtime, TOKEN_LEFT_BRACKET)) depth++;
        else if (match(runtime, TOKEN_RIGHT_BRACKET)) depth--;
        else advance(runtime);
    }
    function->body_end = runtime->current - 1;
    function->next = runtime->functions;
    runtime->functions = function;
}
static Value call_user_function(Runtime *runtime, Function *function, Value *arguments, size_t count) {
    if (count != function->parameter_count) { error_at(runtime, "wrong number of function arguments"); return null_value(); }
    Variable *saved_variables = runtime->variables;
    size_t saved_current = runtime->current;
    int saved_return_signal = runtime->return_signal;
    Value saved_return_value = runtime->return_value;
    runtime->variables = NULL; runtime->return_signal = 0; runtime->return_value = null_value();
    for (size_t i = 0; i < count; i++) set_variable(runtime, function->parameters[i], arguments[i], 0);
    runtime->current = function->body_start - 1;
    execute_block(runtime);
    Value result = copy_value(runtime->return_value);
    while (runtime->variables) { Variable *next = runtime->variables->next; free(runtime->variables->name); free_value(&runtime->variables->value); free(runtime->variables); runtime->variables = next; }
    free_value(&runtime->return_value);
    runtime->return_value = saved_return_value; runtime->return_signal = saved_return_signal;
    runtime->variables = saved_variables; runtime->current = saved_current;
    return result;
}
static void execute_if(Runtime *runtime) {
    Value condition = expression(runtime);
    int enabled = truthy(condition); free_value(&condition);
    skip_lines(runtime);
    if (enabled) execute_block(runtime); else skip_block(runtime);
    skip_lines(runtime);
    if (match(runtime, TOKEN_ELSE)) {
        skip_lines(runtime);
        if (match(runtime, TOKEN_IF)) {
            if (enabled) { skip_block(runtime); }
            else execute_if(runtime);
        } else if (enabled) skip_block(runtime); else execute_block(runtime);
    }
}
static void execute_while(Runtime *runtime, int infinite) {
    size_t condition_start = runtime->current;
    Value condition = infinite ? bool_value(1) : expression(runtime);
    int enabled = truthy(condition); free_value(&condition);
    skip_lines(runtime); size_t block_start = runtime->current;
    while (enabled && !runtime->failed) {
        runtime->break_signal = runtime->continue_signal = 0;
        execute_block(runtime);
        if (runtime->break_signal || runtime->return_signal) { runtime->break_signal = 0; break; }
        runtime->current = condition_start;
        condition = infinite ? bool_value(1) : expression(runtime);
        enabled = truthy(condition); free_value(&condition);
        skip_lines(runtime); block_start = runtime->current;
    }
    if (!enabled && runtime->current != block_start) runtime->current = block_start;
    if (!enabled) skip_block(runtime);
}
static void execute_statement(Runtime *runtime) {
    skip_lines(runtime);
    if (match(runtime, TOKEN_FUNC)) { declare_function(runtime); return; }
    int is_const = match(runtime, TOKEN_CONST);
    if (is_const || match(runtime, TOKEN_LET)) {
        Token *name = peek(runtime);
        if (!match(runtime, TOKEN_IDENTIFIER) || !match(runtime, TOKEN_EQUAL)) { error_at(runtime, "expected variable name and ="); return; }
        Value value = expression(runtime); set_variable(runtime, name->lexeme, value, is_const); free_value(&value); return;
    }
    if (match(runtime, TOKEN_IF)) { execute_if(runtime); return; }
    if (match(runtime, TOKEN_WHILE)) { execute_while(runtime, 0); return; }
    if (match(runtime, TOKEN_LOOP)) { execute_while(runtime, 1); return; }
    if (match(runtime, TOKEN_BREAK)) { runtime->break_signal = 1; return; }
    if (match(runtime, TOKEN_CONTINUE)) { runtime->continue_signal = 1; return; }
    if (match(runtime, TOKEN_RETURN)) {
        Value value = peek(runtime)->type == TOKEN_NEWLINE || peek(runtime)->type == TOKEN_RIGHT_BRACKET ? null_value() : expression(runtime);
        free_value(&runtime->return_value); runtime->return_value = value; runtime->return_signal = 1; return;
    }
    Value value = expression(runtime); free_value(&value);
}

int runtime_run(TokenList *tokens) {
    Runtime runtime = {tokens, 0, NULL, NULL, 0, 0, 0, 0, null_value()};
    while (peek(&runtime)->type != TOKEN_EOF && !runtime.failed) {
        execute_statement(&runtime);
        skip_lines(&runtime);
    }
    while (runtime.variables) { Variable *next = runtime.variables->next; free(runtime.variables->name); free_value(&runtime.variables->value); free(runtime.variables); runtime.variables = next; }
    while (runtime.functions) { Function *next = runtime.functions->next; for (size_t i = 0; i < runtime.functions->parameter_count; i++) free(runtime.functions->parameters[i]); free(runtime.functions->parameters); free(runtime.functions->name); free(runtime.functions); runtime.functions = next; }
    free_value(&runtime.return_value);
    return runtime.failed ? 1 : 0;
}