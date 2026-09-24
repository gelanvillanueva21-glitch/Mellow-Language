#include "runtime.h"
#include "runtime/model.h"
#include "utils/file.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Variable *find_variable(Runtime *runtime, const char *name);
static Field *instance_field(Instance *instance, const char *name);

static char *duplicate_text(const char *text) {
    size_t length = strlen(text) + 1;
    char *copy = malloc(length);
    if (copy) memcpy(copy, text, length);
    return copy;
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
static Token *peek(Runtime *runtime) { return &runtime->tokens->items[runtime->current]; }
static Token *advance(Runtime *runtime) { return &runtime->tokens->items[runtime->current++]; }
static int match(Runtime *runtime, TokenType type) { if (peek(runtime)->type != type) return 0; advance(runtime); return 1; }
static void error_at(Runtime *runtime, const char *message) {
    Token *token = peek(runtime);
    fprintf(stderr, "Mellow error at %d:%d: %s\n", token->line, token->column, message);
    free(runtime->error_message);
    runtime->error_message = duplicate_text(message);
    runtime->failed = 1;
}
static void error_message(Runtime *runtime, const char *message) {
    free(runtime->error_message);
    runtime->error_message = duplicate_text(message);
    runtime->failed = 1;
}
static Variable *find_variable(Runtime *runtime, const char *name) {
    for (Variable *variable = runtime->variables; variable; variable = variable->next)
        if (strcmp(variable->name, name) == 0) return variable;
    Field *field = instance_field(runtime->this_instance, name);
    if (field) {
        static Variable result;
        result.name = field->name; result.value = field->value; result.constant = 0; result.next = NULL;
        return &result;
    }
    return NULL;
}
static Function *find_function(Runtime *runtime, const char *name) {
    for (Function *function = runtime->functions; function; function = function->next)
        if (strcmp(function->name, name) == 0) return function;
    return NULL;
}
static Class *find_class(Runtime *runtime, const char *name) {
    for (Class *class_info = runtime->classes; class_info; class_info = class_info->next)
        if (strcmp(class_info->name, name) == 0) return class_info;
    return NULL;
}
static Field *find_field(Field *fields, const char *name) {
    for (Field *field = fields; field; field = field->next)
        if (strcmp(field->name, name) == 0) return field;
    return NULL;
}
static Function *find_method(Class *class_info, const char *name) {
    for (Class *current = class_info; current; current = current->parent)
        for (Function *method = current->methods; method; method = method->next)
            if (strcmp(method->name, name) == 0) return method;
    return NULL;
}
static int is_descendant(Class *class_info, Class *ancestor) {
    for (Class *current = class_info; current; current = current->parent)
        if (current == ancestor) return 1;
    return 0;
}
static int selected_import(Runtime *runtime, const char *name) {
    if (!runtime->importing_module || runtime->import_name_count == 0) return 1;
    for (size_t i = 0; i < runtime->import_name_count; i++)
        if (strcmp(runtime->import_names[i], name) == 0) return 1;
    return 0;
}
static int can_access_method(Runtime *runtime, Function *method) {
    return !method->is_private || (runtime->active_class && is_descendant(runtime->active_class, method->owner));
}
static Field *instance_field(Instance *instance, const char *name) {
    return instance ? find_field(instance->fields, name) : NULL;
}
static Value instance_value(Instance *instance) {
    return (Value){VALUE_INSTANCE, 0, 0, NULL, NULL, NULL, instance};
}
static void copy_class_fields(Class *class_info, Instance *instance) {
    if (class_info->parent) copy_class_fields(class_info->parent, instance);
    for (Field *field = class_info->fields; field; field = field->next) {
        Field *copy = calloc(1, sizeof(*copy));
        copy->name = duplicate_text(field->name); copy->value = copy_value(field->value);
        copy->next = instance->fields; instance->fields = copy;
    }
}
static Instance *new_instance(Class *class_info) {
    Instance *instance = calloc(1, sizeof(*instance));
    instance->class_info = class_info; copy_class_fields(class_info, instance);
    return instance;
}
static void free_instance(Instance *instance) {
    if (!instance) return;
    Field *field = instance->fields;
    while (field) { Field *next = field->next; free(field->name); free_value(&field->value); free(field); field = next; }
    free(instance);
}
static void set_variable(Runtime *runtime, const char *name, Value value, int constant) {
    Field *field = instance_field(runtime->this_instance, name);
    Variable *local = NULL;
    for (Variable *candidate = runtime->variables; candidate; candidate = candidate->next)
        if (strcmp(candidate->name, name) == 0) { local = candidate; break; }
    if (field && !local) {
        free_value(&field->value); field->value = copy_value(value); return;
    }
    Variable *variable = local;
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
static void execute_statement(Runtime *runtime);
static void execute_program(Runtime *runtime);
static size_t parse_arguments(Runtime *runtime, Value *arguments) {
    size_t count = 0;
    skip_lines(runtime);
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
    return count;
}

static int is_number_pair(Value *arguments, size_t count) {
    return count == 2 && arguments[0].type == VALUE_NUMBER && arguments[1].type == VALUE_NUMBER;
}
static int named_as(const char *name, const char *short_name, const char *long_name) {
    return strcmp(name, short_name) == 0 || strcmp(name, long_name) == 0;
}
static char *text_range(const char *start, size_t length) {
    char *text = malloc(length + 1);
    if (!text) return NULL;
    memcpy(text, start, length); text[length] = '\0';
    return text;
}
static Value string_trim(const char *text) {
    const char *start = text;
    while (*start && isspace((unsigned char)*start)) start++;
    const char *end = text + strlen(text);
    while (end > start && isspace((unsigned char)end[-1])) end--;
    char *trimmed = text_range(start, (size_t)(end - start));
    Value result = string_value(trimmed ? trimmed : ""); free(trimmed); return result;
}
static Value string_case(const char *text, int uppercase) {
    char *result = duplicate_text(text);
    if (!result) return string_value("");
    for (char *current = result; *current; current++)
        *current = (char)(uppercase ? toupper((unsigned char)*current) : tolower((unsigned char)*current));
    Value value = string_value(result); free(result); return value;
}
static Value string_replace(const char *text, const char *old_text, const char *new_text) {
    if (old_text[0] == '\0') return string_value(text);
    size_t old_length = strlen(old_text), new_length = strlen(new_text), occurrences = 0;
    const char *scan = text;
    while ((scan = strstr(scan, old_text)) != NULL) { occurrences++; scan += old_length; }
    size_t text_length = strlen(text);
    size_t result_length = text_length + occurrences * (new_length - old_length);
    char *result = malloc(result_length + 1);
    if (!result) return string_value("");
    const char *source = text; char *destination = result;
    while ((scan = strstr(source, old_text)) != NULL) {
        size_t prefix = (size_t)(scan - source);
        memcpy(destination, source, prefix); destination += prefix;
        memcpy(destination, new_text, new_length); destination += new_length;
        source = scan + old_length;
    }
    strcpy(destination, source);
    Value value = string_value(result); free(result); return value;
}
static Value string_split(const char *text, const char *delimiter) {
    Value result = collection_value(VALUE_LIST);
    size_t delimiter_length = strlen(delimiter);
    if (delimiter_length == 0) { Value item = string_value(text); collection_append(result, item); free_value(&item); return result; }
    const char *start = text; const char *match_position;
    while ((match_position = strstr(start, delimiter)) != NULL) {
        char *part = text_range(start, (size_t)(match_position - start));
        Value item = string_value(part ? part : "");
        free(part); collection_append(result, item); free_value(&item);
        start = match_position + delimiter_length;
    }
    Value item = string_value(start); collection_append(result, item); free_value(&item);
    return result;
}
static Value string_join(Value list, const char *delimiter) {
    size_t delimiter_length = strlen(delimiter), length = 1;
    for (size_t i = 0; i < list.collection->count; i++) {
        if (list.collection->items[i].type != VALUE_STRING) return null_value();
        length += strlen(list.collection->items[i].string);
        if (i) length += delimiter_length;
    }
    char *result = calloc(length, 1);
    if (!result) return string_value("");
    for (size_t i = 0; i < list.collection->count; i++) {
        if (i) strcat(result, delimiter);
        strcat(result, list.collection->items[i].string);
    }
    Value value = string_value(result); free(result); return value;
}
static Value call_builtin(Runtime *runtime, const char *name, Value *arguments, size_t count) {
    if (strcmp(name, "input") == 0 && (count == 0 || (count == 1 && arguments[0].type == VALUE_STRING))) {
        if (count == 1) { fputs(arguments[0].string, stdout); fflush(stdout); }
        char buffer[4096];
        if (!fgets(buffer, sizeof(buffer), stdin)) return string_value("");
        buffer[strcspn(buffer, "\r\n")] = '\0';
        return string_value(buffer);
    }
    if ((strcmp(name, "trim") == 0 || strcmp(name, "upper") == 0 || strcmp(name, "lower") == 0) && count == 1) {
        if (arguments[0].type != VALUE_STRING) { error_at(runtime, "string function expects a string"); return null_value(); }
        if (strcmp(name, "trim") == 0) return string_trim(arguments[0].string);
        return string_case(arguments[0].string, strcmp(name, "upper") == 0);
    }
    if (strcmp(name, "replace") == 0 && count == 3) {
        if (arguments[0].type != VALUE_STRING || arguments[1].type != VALUE_STRING || arguments[2].type != VALUE_STRING) { error_at(runtime, "replace expects three strings"); return null_value(); }
        return string_replace(arguments[0].string, arguments[1].string, arguments[2].string);
    }
    if ((strcmp(name, "starts_with") == 0 || strcmp(name, "ends_with") == 0) && count == 2) {
        if (arguments[0].type != VALUE_STRING || arguments[1].type != VALUE_STRING) { error_at(runtime, "string predicate expects two strings"); return null_value(); }
        if (strcmp(name, "starts_with") == 0) return bool_value(strncmp(arguments[0].string, arguments[1].string, strlen(arguments[1].string)) == 0);
        size_t text_length = strlen(arguments[0].string), suffix_length = strlen(arguments[1].string);
        return bool_value(suffix_length <= text_length && strcmp(arguments[0].string + text_length - suffix_length, arguments[1].string) == 0);
    }
    if (strcmp(name, "split") == 0 && count == 2) {
        if (arguments[0].type != VALUE_STRING || arguments[1].type != VALUE_STRING) { error_at(runtime, "split expects two strings"); return null_value(); }
        return string_split(arguments[0].string, arguments[1].string);
    }
    if (strcmp(name, "join") == 0 && count == 2) {
        if ((arguments[0].type != VALUE_LIST && arguments[0].type != VALUE_ARRAY) || arguments[1].type != VALUE_STRING) { error_at(runtime, "join expects a collection and a string delimiter"); return null_value(); }
        Value result = string_join(arguments[0], arguments[1].string);
        if (result.type == VALUE_NULL) { error_at(runtime, "join expects a collection of strings"); return null_value(); }
        return result;
    }
    if (strcmp(name, "print") == 0 || strcmp(name, "Print") == 0) {
        for (size_t i = 0; i < count; i++) {
            if (i) putchar(' ');
            print_value(arguments[i]);
        }
        putchar('\n'); return null_value();
    }
    if (strcmp(name, "raise") == 0 && count == 1) { 
        runtime->exception_raised = 1;
        if (arguments[0].type == VALUE_STRING) error_message(runtime, arguments[0].string);
        else error_message(runtime, "raised non-string value");
        return null_value();
    }
    if (strcmp(name, "destroy") == 0 && count == 1 && arguments[0].type == VALUE_INSTANCE) {
        Instance *instance = arguments[0].instance;
        Function *destructor = find_method(instance->class_info, "free");
        if (destructor) {
            Instance *saved_this = runtime->this_instance; runtime->this_instance = instance;
            call_user_function(runtime, destructor, NULL, 0);
            runtime->this_instance = saved_this;
        }
        free_instance(instance); return null_value();
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
    if (named_as(name, "eq", "equal") && count == 2) return bool_value(value_equal(arguments[0], arguments[1]));
    if (named_as(name, "neq", "not_equal") && count == 2) return bool_value(!value_equal(arguments[0], arguments[1]));
    if (is_number_pair(arguments, count) && (named_as(name, "lt", "less") || named_as(name, "gt", "greater") || named_as(name, "lte", "less_equal") || named_as(name, "gte", "greater_equal"))) {
        if (named_as(name, "lt", "less")) return bool_value(arguments[0].number < arguments[1].number);
        if (named_as(name, "gt", "greater")) return bool_value(arguments[0].number > arguments[1].number);
        if (named_as(name, "lte", "less_equal")) return bool_value(arguments[0].number <= arguments[1].number);
        return bool_value(arguments[0].number >= arguments[1].number);
    }
    if (strcmp(name, "and") == 0 && count == 2) return bool_value(arguments[0].boolean && arguments[1].boolean);
    if (strcmp(name, "or") == 0 && count == 2) return bool_value(arguments[0].boolean || arguments[1].boolean);
    if (strcmp(name, "not") == 0 && count == 1) return bool_value(!arguments[0].boolean);
    if ((strcmp(name, "abs") == 0 || strcmp(name, "floor") == 0 || strcmp(name, "ceil") == 0 || strcmp(name, "round") == 0) && count == 1) {
        if (arguments[0].type != VALUE_NUMBER) { error_at(runtime, "numeric builtin expects a number"); return null_value(); }
        if (strcmp(name, "abs") == 0) return number_value(fabs(arguments[0].number));
        if (strcmp(name, "floor") == 0) return number_value(floor(arguments[0].number));
        if (strcmp(name, "ceil") == 0) return number_value(ceil(arguments[0].number));
        return number_value(round(arguments[0].number));
    }
    if ((strcmp(name, "min") == 0 || strcmp(name, "max") == 0) && count == 2) {
        if (!is_number_pair(arguments, count)) { error_at(runtime, "min and max expect numbers"); return null_value(); }
        return number_value(strcmp(name, "min") == 0 ? fmin(arguments[0].number, arguments[1].number) : fmax(arguments[0].number, arguments[1].number));
    }
    if (strcmp(name, "clamp") == 0 && count == 3) {
        if (arguments[0].type != VALUE_NUMBER || arguments[1].type != VALUE_NUMBER || arguments[2].type != VALUE_NUMBER) { error_at(runtime, "clamp expects numbers"); return null_value(); }
        return number_value(fmin(fmax(arguments[0].number, arguments[1].number), arguments[2].number));
    }
    if (strcmp(name, "range") == 0 && (count == 2 || count == 3)) {
        if (arguments[0].type != VALUE_NUMBER || arguments[1].type != VALUE_NUMBER || (count == 3 && arguments[2].type != VALUE_NUMBER)) { error_at(runtime, "range expects numbers"); return null_value(); }
        double step = count == 3 ? arguments[2].number : 1;
        if (step == 0) { error_at(runtime, "range step cannot be zero"); return null_value(); }
        Value result = collection_value(VALUE_LIST);
        for (double value = arguments[0].number; step > 0 ? value < arguments[1].number : value > arguments[1].number; value += step) collection_append(result, number_value(value));
        return result;
    }
    if ((strcmp(name, "get") == 0 || strcmp(name, "has") == 0) && count == 2) {
        Value container = arguments[0];
        if (container.type == VALUE_DICT && arguments[1].type == VALUE_STRING) {
            Value *found = dictionary_get(container, arguments[1].string);
            return strcmp(name, "has") == 0 ? bool_value(found != NULL) : (found ? copy_value(*found) : null_value());
        }
        if ((container.type == VALUE_ARRAY || container.type == VALUE_LIST) && arguments[1].type == VALUE_NUMBER) {
            long index = (long)arguments[1].number;
            if (index < 0 || (size_t)index >= container.collection->count) return strcmp(name, "has") == 0 ? bool_value(0) : null_value();
            return strcmp(name, "has") == 0 ? bool_value(1) : copy_value(container.collection->items[index]);
        }
        error_at(runtime, "get/has expects a dictionary key or collection index"); return null_value();
    }
    if (strcmp(name, "put") == 0 && count == 3 && arguments[0].type == VALUE_DICT && arguments[1].type == VALUE_STRING) {
        dictionary_set(&arguments[0], arguments[1].string, arguments[2]);
        return null_value();
    }
    if ((strcmp(name, "is_null") == 0 || strcmp(name, "is_number") == 0 || strcmp(name, "is_string") == 0 || strcmp(name, "is_array") == 0 || strcmp(name, "is_list") == 0) && count == 1) {
        ValueType type = arguments[0].type;
        if (strcmp(name, "is_null") == 0) return bool_value(type == VALUE_NULL);
        if (strcmp(name, "is_number") == 0) return bool_value(type == VALUE_NUMBER);
        if (strcmp(name, "is_string") == 0) return bool_value(type == VALUE_STRING);
        if (strcmp(name, "is_array") == 0) return bool_value(type == VALUE_ARRAY);
        return bool_value(type == VALUE_LIST);
    }
    if ((strcmp(name, "abs") == 0 || strcmp(name, "floor") == 0 || strcmp(name, "ceil") == 0 || strcmp(name, "round") == 0) && count == 1) {
        if (arguments[0].type != VALUE_NUMBER) { error_at(runtime, "numeric builtin expects a number"); return null_value(); }
        if (strcmp(name, "abs") == 0) return number_value(fabs(arguments[0].number));
        if (strcmp(name, "floor") == 0) return number_value(floor(arguments[0].number));
        if (strcmp(name, "ceil") == 0) return number_value(ceil(arguments[0].number));
        return number_value(round(arguments[0].number));
    }
    if ((strcmp(name, "min") == 0 || strcmp(name, "max") == 0) && count == 2) {
        if (!is_number_pair(arguments, count)) { error_at(runtime, "min and max expect numbers"); return null_value(); }
        return number_value(strcmp(name, "min") == 0 ? fmin(arguments[0].number, arguments[1].number) : fmax(arguments[0].number, arguments[1].number));
    }
    if (strcmp(name, "clamp") == 0 && count == 3) {
        if (arguments[0].type != VALUE_NUMBER || arguments[1].type != VALUE_NUMBER || arguments[2].type != VALUE_NUMBER) { error_at(runtime, "clamp expects numbers"); return null_value(); }
        return number_value(fmin(fmax(arguments[0].number, arguments[1].number), arguments[2].number));
    }
    if ((strcmp(name, "is_null") == 0 || strcmp(name, "is_number") == 0 || strcmp(name, "is_string") == 0 || strcmp(name, "is_array") == 0 || strcmp(name, "is_list") == 0) && count == 1) {
        ValueType type = arguments[0].type;
        if (strcmp(name, "is_null") == 0) return bool_value(type == VALUE_NULL);
        if (strcmp(name, "is_number") == 0) return bool_value(type == VALUE_NUMBER);
        if (strcmp(name, "is_string") == 0) return bool_value(type == VALUE_STRING);
        if (strcmp(name, "is_array") == 0) return bool_value(type == VALUE_ARRAY);
        return bool_value(type == VALUE_LIST);
    }
    if ((strcmp(name, "inside") == 0 || strcmp(name, "contains") == 0) && count == 2) {
        Value needle = arguments[0];
        Value container = arguments[1];
        if (container.type == VALUE_STRING) {
            if (needle.type != VALUE_STRING) { error_at(runtime, "inside expects a string value for a string container"); return null_value(); }
            return bool_value(strstr(container.string, needle.string) != NULL);
        }
        if (container.type == VALUE_ARRAY || container.type == VALUE_LIST) {
            for (size_t i = 0; i < container.collection->count; i++)
                if (value_equal(needle, container.collection->items[i])) return bool_value(1);
            return bool_value(0);
        }
        if (container.type == VALUE_DICT && needle.type == VALUE_STRING) return bool_value(dictionary_has(container, needle.string));
        error_at(runtime, "inside expects a string, array, or list container");
        return null_value();
    }
    if (strcmp(name, "type") == 0 && count == 1) return string_value(value_type_name(arguments[0]));
    if ((strcmp(name, "to_number") == 0 || strcmp(name, "to_float") == 0 || strcmp(name, "to_int") == 0) && count == 1) {
        if (arguments[0].type == VALUE_NUMBER) return strcmp(name, "to_int") == 0 ? number_value(trunc(arguments[0].number)) : copy_value(arguments[0]);
        if (arguments[0].type != VALUE_STRING) { error_at(runtime, "numeric conversion expects a string or number"); return null_value(); }
        char *end = NULL; double number = strtod(arguments[0].string, &end);
        while (end && (*end == ' ' || *end == '\t')) end++;
        if (end == arguments[0].string || (end && *end != '\0')) { error_at(runtime, "invalid numeric input"); return null_value(); }
        return strcmp(name, "to_int") == 0 ? number_value(trunc(number)) : number_value(number);
    }
    if (strcmp(name, "to_bool") == 0 && count == 1) {
        if (arguments[0].type == VALUE_BOOL) return copy_value(arguments[0]);
        if (arguments[0].type == VALUE_NUMBER) return bool_value(arguments[0].number != 0);
        if (arguments[0].type == VALUE_STRING) {
            if (strcmp(arguments[0].string, "true") == 0 || strcmp(arguments[0].string, "1") == 0) return bool_value(1);
            if (strcmp(arguments[0].string, "false") == 0 || strcmp(arguments[0].string, "0") == 0) return bool_value(0);
        }
        error_at(runtime, "invalid boolean input; use true, false, 1, or 0"); return null_value();
    }
    if (strcmp(name, "list") == 0) {
        Value result = collection_value(VALUE_LIST);
        for (size_t i = 0; i < count; i++) collection_append(result, arguments[i]);
        return result;
    }
    if (strcmp(name, "len") == 0 && arguments[0].type == VALUE_STRING) return number_value((double)strlen(arguments[0].string));
    if (strcmp(name, "len") == 0 && (arguments[0].type == VALUE_ARRAY || arguments[0].type == VALUE_LIST)) return number_value((double)arguments[0].collection->count);
    if (strcmp(name, "len") == 0 && arguments[0].type == VALUE_DICT) { size_t count = 0; for (DictEntry *entry = arguments[0].dictionary; entry; entry = entry->next) count++; return number_value((double)count); }
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
            skip_lines(runtime);
            Value first = expression(runtime);
            skip_lines(runtime);
            if (match(runtime, TOKEN_COLON)) {
                free(result.collection);
                result = dictionary_value();
                if (first.type != VALUE_STRING) { error_at(runtime, "dictionary keys must be strings"); free_value(&first); return null_value(); }
                Value value = expression(runtime); dictionary_set(&result, first.string, value); free_value(&first); free_value(&value);
                while (match(runtime, TOKEN_COMMA)) {
                    skip_lines(runtime);
                    Value key = expression(runtime); skip_lines(runtime);
                    if (!match(runtime, TOKEN_COLON) || key.type != VALUE_STRING) { error_at(runtime, "expected string dictionary key and colon"); free_value(&key); break; }
                    Value item = expression(runtime); dictionary_set(&result, key.string, item); free_value(&key); free_value(&item); skip_lines(runtime);
                }
            } else {
                collection_append(result, first); free_value(&first);
                while (match(runtime, TOKEN_COMMA)) { skip_lines(runtime); Value item = expression(runtime); collection_append(result, item); free_value(&item); skip_lines(runtime); }
            }
            skip_lines(runtime);
            if (!match(runtime, TOKEN_RIGHT_BRACE)) error_at(runtime, "expected } after collection literal");
        }
        return result;
    }
    if (token->type == TOKEN_THIS) {
        if (match(runtime, TOKEN_DOT)) {
            Token *member = peek(runtime);
            if (!match(runtime, TOKEN_IDENTIFIER)) { error_at(runtime, "expected this member name"); return null_value(); }
            if (match(runtime, TOKEN_LEFT_ANGLE)) {
                Value arguments[32]; size_t count = parse_arguments(runtime, arguments);
                Function *method = runtime->this_instance ? find_method(runtime->this_instance->class_info, member->lexeme) : NULL;
                Value result = method && can_access_method(runtime, method) ? call_user_function(runtime, method, arguments, count) : null_value();
                if (!method) error_at(runtime, "unknown this method");
                else if (!can_access_method(runtime, method)) error_at(runtime, "private method is not accessible here");
                for (size_t i = 0; i < count; i++) free_value(&arguments[i]);
                return result;
            }
            Field *field = instance_field(runtime->this_instance, member->lexeme);
            if (!field) { error_at(runtime, "unknown this field"); return null_value(); }
            return copy_value(field->value);
        }
        return instance_value(runtime->this_instance);
    }
    if (token->type == TOKEN_SUPER) {
        if (!match(runtime, TOKEN_DOT)) { error_at(runtime, "expected . after super"); return null_value(); }
        Token *method_name = peek(runtime);
        if (!match(runtime, TOKEN_IDENTIFIER) || !match(runtime, TOKEN_LEFT_ANGLE)) { error_at(runtime, "expected super method call"); return null_value(); }
        Value arguments[32]; size_t count = parse_arguments(runtime, arguments);
        Function *method = runtime->this_instance ? find_method(runtime->this_instance->class_info->parent, method_name->lexeme) : NULL;
        Value result = method && can_access_method(runtime, method) ? call_user_function(runtime, method, arguments, count) : null_value();
        if (!method) error_at(runtime, "unknown super method");
        else if (!can_access_method(runtime, method)) error_at(runtime, "private method is not accessible here");
        for (size_t i = 0; i < count; i++) free_value(&arguments[i]);
        return result;
    }
    if (token->type == TOKEN_IDENTIFIER || token->type == TOKEN_RAISE || token->type == TOKEN_AND || token->type == TOKEN_OR || token->type == TOKEN_NOT) {
        if (match(runtime, TOKEN_DOT)) {
            Token *member = peek(runtime);
            if (!match(runtime, TOKEN_IDENTIFIER)) { error_at(runtime, "expected member name"); return null_value(); }
            Variable *base = find_variable(runtime, token->lexeme);
            if (!base || base->value.type != VALUE_INSTANCE) { error_at(runtime, "member access requires an instance"); return null_value(); }
            Instance *instance = base->value.instance;
            if (match(runtime, TOKEN_LEFT_ANGLE)) {
                Value arguments[32]; size_t count = parse_arguments(runtime, arguments);
                Function *method = find_method(instance->class_info, member->lexeme);
                Instance *saved_this = runtime->this_instance; runtime->this_instance = instance;
                Value result = method && can_access_method(runtime, method) ? call_user_function(runtime, method, arguments, count) : null_value();
                runtime->this_instance = saved_this;
                if (!method) error_at(runtime, "unknown instance method");
                else if (!can_access_method(runtime, method)) error_at(runtime, "private method is not accessible here");
                for (size_t i = 0; i < count; i++) free_value(&arguments[i]);
                return result;
            }
            Field *field = instance_field(instance, member->lexeme);
            if (!field) { error_at(runtime, "unknown instance field"); return null_value(); }
            return copy_value(field->value);
        }
        if (match(runtime, TOKEN_LEFT_ANGLE)) {
            Value arguments[32]; size_t count = 0;
            skip_lines(runtime);
            if (strcmp(token->lexeme, "set") == 0) {
                Token *name = peek(runtime);
                char *field_name = NULL;
                if (match(runtime, TOKEN_THIS)) {
                    if (!match(runtime, TOKEN_DOT)) { error_at(runtime, "set expects this.field"); return null_value(); }
                    Token *member = peek(runtime);
                    if (!match(runtime, TOKEN_IDENTIFIER)) { error_at(runtime, "set expects a field name"); return null_value(); }
                    field_name = member->lexeme;
                } else if (!match(runtime, TOKEN_IDENTIFIER)) { error_at(runtime, "set expects a variable name"); return null_value(); }
                else field_name = name->lexeme;
                skip_lines(runtime);
                if (!match(runtime, TOKEN_COMMA)) { error_at(runtime, "set expects a value"); return null_value(); }
                Value value = expression(runtime);
                skip_lines(runtime);
                if (!match(runtime, TOKEN_RIGHT_ANGLE)) error_at(runtime, "expected > after set");
                Variable *variable = find_variable(runtime, field_name);
                if (!variable) error_at(runtime, "cannot set an unknown variable");
                else if (variable->constant) error_at(runtime, "cannot set a const variable");
                else {
                    Field *field = instance_field(runtime->this_instance, field_name);
                    if (field) { free_value(&field->value); field->value = copy_value(value); }
                    else { free_value(&variable->value); variable->value = copy_value(value); }
                }
                free_value(&value);
                return null_value();
            }
            if (strcmp(token->lexeme, "put") == 0) {
                Token *name = peek(runtime);
                if (!match(runtime, TOKEN_IDENTIFIER)) { error_at(runtime, "put expects a dictionary variable"); return null_value(); }
                if (!match(runtime, TOKEN_COMMA)) { error_at(runtime, "put expects a key"); return null_value(); }
                Value key = expression(runtime);
                if (key.type != VALUE_STRING) { error_at(runtime, "dictionary keys must be strings"); free_value(&key); return null_value(); }
                if (!match(runtime, TOKEN_COMMA)) { error_at(runtime, "put expects a value"); free_value(&key); return null_value(); }
                Value value = expression(runtime); skip_lines(runtime);
                if (!match(runtime, TOKEN_RIGHT_ANGLE)) error_at(runtime, "expected > after put");
                Variable *variable = find_variable(runtime, name->lexeme);
                if (!variable || variable->value.type != VALUE_DICT) error_at(runtime, "put expects a dictionary variable");
                else dictionary_set(&variable->value, key.string, value);
                free_value(&key); free_value(&value); return null_value();
            }
            count = parse_arguments(runtime, arguments);
            Function *function = find_function(runtime, token->lexeme);
            Class *class_info = find_class(runtime, token->lexeme);
            Value result;
            if (class_info) {
                if (count == 0 && !find_method(class_info, "init")) result = instance_value(new_instance(class_info));
                else {
                    Instance *instance = new_instance(class_info); Instance *saved_this = runtime->this_instance; runtime->this_instance = instance;
                    Function *constructor = find_method(class_info, "init");
                    if (!constructor) { error_at(runtime, "constructor init not found"); result = null_value(); }
                    else { call_user_function(runtime, constructor, arguments, count); result = instance_value(instance); }
                    runtime->this_instance = saved_this;
                }
            } else result = function ? call_user_function(runtime, function, arguments, count) : call_builtin(runtime, token->lexeme, arguments, count);
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
    if (value.type == VALUE_DICT) return value.dictionary != NULL;
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
static void remember_module(Runtime *runtime, TokenList *tokens) {
    if (runtime->module_count == runtime->module_capacity) {
        size_t capacity = runtime->module_capacity == 0 ? 4 : runtime->module_capacity * 2;
        runtime->modules = realloc(runtime->modules, capacity * sizeof(*runtime->modules));
        runtime->module_capacity = capacity;
    }
    runtime->modules[runtime->module_count++] = tokens;
}
static void execute_import(Runtime *runtime) {
    Token *path = peek(runtime);
    if (!match(runtime, TOKEN_STRING)) { error_at(runtime, "import expects a quoted file path"); return; }
    char **saved_names = runtime->import_names;
    size_t saved_count = runtime->import_name_count;
    int saved_importing = runtime->importing_module;
    runtime->import_names = NULL; runtime->import_name_count = 0;
    if (match(runtime, TOKEN_LEFT_ANGLE)) {
        if (!match(runtime, TOKEN_RIGHT_ANGLE)) {
            do {
                Token *name = peek(runtime);
                if (!match(runtime, TOKEN_IDENTIFIER)) { error_at(runtime, "import selectors must be function or class names"); break; }
                runtime->import_names = realloc(runtime->import_names, (runtime->import_name_count + 1) * sizeof(char *));
                runtime->import_names[runtime->import_name_count++] = duplicate_text(name->lexeme);
            } while (match(runtime, TOKEN_COMMA));
            if (!match(runtime, TOKEN_RIGHT_ANGLE)) error_at(runtime, "expected > after import names");
        }
    }
    char *source = read_file(path->lexeme);
    if (!source) { error_at(runtime, "could not load imported module"); return; }
    TokenList *module = malloc(sizeof(*module));
    if (!module) { free(source); error_at(runtime, "could not allocate imported module"); return; }
    *module = lexer_scan(source); free(source);
    remember_module(runtime, module);
    TokenList *saved_tokens = runtime->tokens;
    size_t saved_current = runtime->current;
    runtime->tokens = module; runtime->current = 0; runtime->importing_module = 1;
    execute_program(runtime);
    runtime->tokens = saved_tokens; runtime->current = saved_current; runtime->importing_module = saved_importing;
    for (size_t i = 0; i < runtime->import_name_count; i++) free(runtime->import_names[i]);
    free(runtime->import_names); runtime->import_names = saved_names; runtime->import_name_count = saved_count;
}
static void skip_import_only_statement(Runtime *runtime) {
    int depth = 0;
    while (peek(runtime)->type != TOKEN_EOF) {
        if (peek(runtime)->type == TOKEN_LEFT_BRACKET) depth++;
        else if (peek(runtime)->type == TOKEN_RIGHT_BRACKET && depth > 0) depth--;
        if (depth == 0 && (peek(runtime)->type == TOKEN_NEWLINE || peek(runtime)->type == TOKEN_SEMICOLON)) break;
        advance(runtime);
    }
}
static void declare_function(Runtime *runtime) {
    Token *name = peek(runtime);
    if (!match(runtime, TOKEN_IDENTIFIER) || !match(runtime, TOKEN_LEFT_ANGLE)) { error_at(runtime, "expected function name and parameters"); return; }
    Function *function = calloc(1, sizeof(*function));
    function->name = duplicate_text(name->lexeme);
    function->token_source = runtime->tokens;
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
    if (selected_import(runtime, function->name)) {
        function->next = runtime->functions;
        runtime->functions = function;
    } else {
        for (size_t i = 0; i < function->parameter_count; i++) free(function->parameters[i]);
        free(function->parameters); free(function->name); free(function);
    }
}
static Function *parse_function_body(Runtime *runtime, const char *name, Class *owner) {
    Function *function = calloc(1, sizeof(*function));
    function->name = duplicate_text(name); function->owner = owner;
    function->token_source = runtime->tokens;
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
    if (!match(runtime, TOKEN_LEFT_BRACKET)) { error_at(runtime, "expected [ after method signature"); return function; }
    function->body_start = runtime->current;
    int depth = 1;
    while (peek(runtime)->type != TOKEN_EOF && depth > 0) {
        if (match(runtime, TOKEN_LEFT_BRACKET)) depth++;
        else if (match(runtime, TOKEN_RIGHT_BRACKET)) depth--;
        else advance(runtime);
    }
    function->body_end = runtime->current - 1;
    return function;
}
static void declare_class(Runtime *runtime) {
    Token *name = peek(runtime);
    if (!match(runtime, TOKEN_IDENTIFIER)) { error_at(runtime, "expected class name"); return; }
    Class *class_info = calloc(1, sizeof(*class_info));
    class_info->name = duplicate_text(name->lexeme);
    if (match(runtime, TOKEN_COLON)) {
        Token *parent = peek(runtime);
        if (!match(runtime, TOKEN_IDENTIFIER)) error_at(runtime, "expected parent class name");
        else class_info->parent = find_class(runtime, parent->lexeme);
        if (!class_info->parent) error_at(runtime, "unknown parent class");
    }
    skip_lines(runtime);
    if (!match(runtime, TOKEN_LEFT_BRACKET)) { error_at(runtime, "expected [ after class name"); free(class_info->name); free(class_info); return; }
    while (peek(runtime)->type != TOKEN_EOF && peek(runtime)->type != TOKEN_RIGHT_BRACKET && !runtime->failed) {
        skip_lines(runtime);
        if (match(runtime, TOKEN_LET)) {
            Token *field_name = peek(runtime);
            if (!match(runtime, TOKEN_IDENTIFIER) || !match(runtime, TOKEN_EQUAL)) { error_at(runtime, "expected field name and ="); break; }
            Value initial = expression(runtime);
            Field *field = calloc(1, sizeof(*field)); field->name = duplicate_text(field_name->lexeme); field->value = initial;
            field->next = class_info->fields; class_info->fields = field;
        } else if (match(runtime, TOKEN_PRIVATE) || match(runtime, TOKEN_FUNC)) {
            int is_private = runtime->tokens->items[runtime->current - 1].type == TOKEN_PRIVATE;
            if (is_private && !match(runtime, TOKEN_FUNC)) { error_at(runtime, "private must be followed by func"); break; }
            Token *method_name = peek(runtime);
            if (!match(runtime, TOKEN_IDENTIFIER) || !match(runtime, TOKEN_LEFT_ANGLE)) { error_at(runtime, "expected method name and parameters"); break; }
            Function *method = parse_function_body(runtime, method_name->lexeme, class_info);
            method->is_private = is_private;
            method->next = class_info->methods; class_info->methods = method;
        } else advance(runtime);
        skip_lines(runtime);
    }
    match(runtime, TOKEN_RIGHT_BRACKET);
    if (selected_import(runtime, class_info->name)) {
        class_info->next = runtime->classes; runtime->classes = class_info;
    } else {
        free(class_info->name); free(class_info);
    }
}
static Value call_user_function(Runtime *runtime, Function *function, Value *arguments, size_t count) {
    if (count != function->parameter_count) { error_at(runtime, "wrong number of function arguments"); return null_value(); }
    Variable *saved_variables = runtime->variables;
    size_t saved_current = runtime->current;
    TokenList *saved_tokens = runtime->tokens;
    int saved_return_signal = runtime->return_signal;
    Value saved_return_value = runtime->return_value;
    Instance *saved_this = runtime->this_instance;
    Class *saved_active_class = runtime->active_class;
    runtime->variables = NULL; runtime->return_signal = 0; runtime->return_value = null_value();
    for (size_t i = 0; i < count; i++) set_variable(runtime, function->parameters[i], arguments[i], 0);
    runtime->current = function->body_start - 1;
    runtime->tokens = function->token_source; runtime->active_class = function->owner;
    execute_block(runtime);
    Value result = copy_value(runtime->return_value);
    while (runtime->variables) { Variable *next = runtime->variables->next; free(runtime->variables->name); free_value(&runtime->variables->value); free(runtime->variables); runtime->variables = next; }
    free_value(&runtime->return_value);
    runtime->return_value = saved_return_value; runtime->return_signal = saved_return_signal;
    runtime->variables = saved_variables; runtime->current = saved_current; runtime->tokens = saved_tokens; runtime->this_instance = saved_this; runtime->active_class = saved_active_class;
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
static void execute_try(Runtime *runtime) {
    execute_block(runtime);
    int caught = runtime->failed;
    char *message = duplicate_text(runtime->error_message ? runtime->error_message : "runtime error");
    skip_lines(runtime);
    if (!match(runtime, TOKEN_CATCH)) {
        if (caught) error_at(runtime, "try requires catch after an error");
        free(message); return;
    }
    skip_lines(runtime);
    if (!match(runtime, TOKEN_LEFT_ANGLE)) { error_at(runtime, "catch expects <error>"); free(message); return; }
    Token *name = peek(runtime);
    if (!match(runtime, TOKEN_IDENTIFIER) || !match(runtime, TOKEN_RIGHT_ANGLE)) { error_at(runtime, "catch expects an error variable"); free(message); return; }
    skip_lines(runtime);
    if (caught) {
        runtime->failed = 0;
        runtime->exception_raised = 0;
        Value error = string_value(message); set_variable(runtime, name->lexeme, error, 0); free_value(&error);
        execute_block(runtime);
    } else skip_block(runtime);
    free(message);
}
static void execute_for(Runtime *runtime) {
    Token *name = peek(runtime);
    if (!match(runtime, TOKEN_IDENTIFIER) || !match(runtime, TOKEN_IN)) { error_at(runtime, "for expects variable in collection"); return; }
    Value collection = expression(runtime);
    if (collection.type != VALUE_ARRAY && collection.type != VALUE_LIST) { error_at(runtime, "for expects an array or list"); free_value(&collection); return; }
    skip_lines(runtime);
    size_t block_start = runtime->current;
    size_t count = collection.collection->count;
    for (size_t i = 0; i < count && !runtime->failed; i++) {
        set_variable(runtime, name->lexeme, collection.collection->items[i], 0);
        runtime->break_signal = runtime->continue_signal = 0;
        runtime->current = block_start;
        execute_block(runtime);
        if (runtime->break_signal) { runtime->break_signal = 0; break; }
        if (runtime->return_signal) break;
    }
    runtime->current = block_start;
    skip_block(runtime);
    free_value(&collection);
}
static void execute_statement(Runtime *runtime) {
    skip_lines(runtime);
    if (runtime->importing_module && peek(runtime)->type != TOKEN_FUNC && peek(runtime)->type != TOKEN_CLASS && peek(runtime)->type != TOKEN_PRIVATE && peek(runtime)->type != TOKEN_IMPORT) {
        skip_import_only_statement(runtime); return;
    }
    if (match(runtime, TOKEN_IMPORT)) { execute_import(runtime); return; }
    if (match(runtime, TOKEN_CLASS)) { declare_class(runtime); return; }
    if (match(runtime, TOKEN_FUNC)) { declare_function(runtime); return; }
    int is_const = match(runtime, TOKEN_CONST);
    if (is_const || match(runtime, TOKEN_LET)) {
        Token *name = peek(runtime);
        if (!match(runtime, TOKEN_IDENTIFIER) || !match(runtime, TOKEN_EQUAL)) { error_at(runtime, "expected variable name and ="); return; }
        Value value = expression(runtime); set_variable(runtime, name->lexeme, value, is_const); free_value(&value); return;
    }
    if (match(runtime, TOKEN_IF)) { execute_if(runtime); return; }
    if (match(runtime, TOKEN_TRY)) { execute_try(runtime); return; }
    if (match(runtime, TOKEN_WHILE)) { execute_while(runtime, 0); return; }
    if (match(runtime, TOKEN_LOOP)) { execute_while(runtime, 1); return; }
    if (match(runtime, TOKEN_FOR)) { execute_for(runtime); return; }
    if (match(runtime, TOKEN_BREAK)) { runtime->break_signal = 1; return; }
    if (match(runtime, TOKEN_CONTINUE)) { runtime->continue_signal = 1; return; }
    if (match(runtime, TOKEN_RETURN)) {
        Value value = peek(runtime)->type == TOKEN_NEWLINE || peek(runtime)->type == TOKEN_RIGHT_BRACKET ? null_value() : expression(runtime);
        free_value(&runtime->return_value); runtime->return_value = value; runtime->return_signal = 1; return;
    }
    Value value = expression(runtime); free_value(&value);
}

static void execute_program(Runtime *runtime) {
    while (peek(runtime)->type != TOKEN_EOF && !runtime->failed) {
        execute_statement(runtime);
        skip_lines(runtime);
    }
}

int runtime_run(TokenList *tokens) {
    Runtime runtime = {.tokens = tokens, .return_value = null_value()};
    execute_program(&runtime);
    if (runtime.failed && runtime.exception_raised)
        fprintf(stderr, "Mellow exception: %s\n", runtime.error_message ? runtime.error_message : "runtime error");
    while (runtime.variables) { Variable *next = runtime.variables->next; free(runtime.variables->name); free_value(&runtime.variables->value); free(runtime.variables); runtime.variables = next; }
    while (runtime.functions) { Function *next = runtime.functions->next; for (size_t i = 0; i < runtime.functions->parameter_count; i++) free(runtime.functions->parameters[i]); free(runtime.functions->parameters); free(runtime.functions->name); free(runtime.functions); runtime.functions = next; }
    for (size_t i = 0; i < runtime.module_count; i++) { token_list_free(runtime.modules[i]); free(runtime.modules[i]); }
    free(runtime.modules);
    free(runtime.error_message);
    free_value(&runtime.return_value);
    return runtime.failed ? 1 : 0;
}