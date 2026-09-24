#include "value.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *duplicate_text(const char *text) {
    size_t length = strlen(text) + 1;
    char *copy = malloc(length);
    if (copy) memcpy(copy, text, length);
    return copy;
}

Value null_value(void) { return (Value){VALUE_NULL, 0, 0, NULL, NULL, NULL, NULL}; }
Value number_value(double number) { return (Value){VALUE_NUMBER, number, 0, NULL, NULL, NULL, NULL}; }
Value bool_value(int boolean) { return (Value){VALUE_BOOL, 0, boolean, NULL, NULL, NULL, NULL}; }

Value string_value(const char *text) {
    return (Value){VALUE_STRING, 0, 0, duplicate_text(text ? text : ""), NULL, NULL, NULL};
}

Value collection_value(ValueType type) {
    return (Value){type, 0, 0, NULL, calloc(1, sizeof(Collection)), NULL, NULL};
}

Value dictionary_value(void) { return (Value){VALUE_DICT, 0, 0, NULL, NULL, NULL, NULL}; }

void free_value(Value *value) {
    if (value->type == VALUE_STRING) free(value->string);
    if ((value->type == VALUE_ARRAY || value->type == VALUE_LIST) && value->collection) {
        for (size_t i = 0; i < value->collection->count; i++) free_value(&value->collection->items[i]);
        free(value->collection->items);
        free(value->collection);
    }
    if (value->type == VALUE_DICT) {
        DictEntry *entry = value->dictionary;
        while (entry) { DictEntry *next = entry->next; free(entry->key); free_value(entry->value); free(entry->value); free(entry); entry = next; }
    }
    value->string = NULL;
    value->collection = NULL;
    value->dictionary = NULL;
    value->instance = NULL;
}

Value copy_value(Value value) {
    if (value.type == VALUE_STRING) return string_value(value.string);
    if (value.type == VALUE_INSTANCE) return value;
    if (value.type == VALUE_DICT) {
        Value copy = dictionary_value();
        for (DictEntry *entry = value.dictionary; entry; entry = entry->next) dictionary_set(&copy, entry->key, *entry->value);
        return copy;
    }
    if (value.type != VALUE_ARRAY && value.type != VALUE_LIST) return value;
    Value copy = collection_value(value.type);
    for (size_t i = 0; i < value.collection->count; i++) collection_append(copy, value.collection->items[i]);
    return copy;
}

int value_equal(Value left, Value right) {
    if (left.type != right.type) return 0;
    if (left.type == VALUE_NUMBER) return left.number == right.number;
    if (left.type == VALUE_STRING) return strcmp(left.string, right.string) == 0;
    if (left.type == VALUE_BOOL) return left.boolean == right.boolean;
    if (left.type == VALUE_NULL) return 1;
    if (left.type == VALUE_INSTANCE) return left.instance == right.instance;
    if (left.type == VALUE_DICT) {
        for (DictEntry *entry = left.dictionary; entry; entry = entry->next) {
            Value *other = dictionary_get(right, entry->key);
            if (!other || !value_equal(*entry->value, *other)) return 0;
        }
        return 1;
    }
    if (left.collection->count != right.collection->count) return 0;
    for (size_t i = 0; i < left.collection->count; i++)
        if (!value_equal(left.collection->items[i], right.collection->items[i])) return 0;
    return 1;
}

const char *value_type_name(Value value) {
    static const char *names[] = {"null", "number", "string", "bool", "array", "list", "dictionary", "instance"};
    return names[value.type];
}

void print_value(Value value) {
    if (value.type == VALUE_STRING) printf("%s", value.string);
    else if (value.type == VALUE_NUMBER) printf("%g", value.number);
    else if (value.type == VALUE_BOOL) printf("%s", value.boolean ? "true" : "false");
    else if (value.type == VALUE_NULL) printf("null");
    else if (value.type == VALUE_INSTANCE) printf("<instance>");
    else if (value.type == VALUE_DICT) {
        putchar('{'); int first = 1;
        for (DictEntry *entry = value.dictionary; entry; entry = entry->next) { if (!first) printf(", "); first = 0; printf("\"%s\": ", entry->key); print_value(*entry->value); }
        putchar('}');
    }
    else {
        putchar('{');
        for (size_t i = 0; i < value.collection->count; i++) {
            if (i) printf(", ");
            print_value(value.collection->items[i]);
        }
        putchar('}');
    }
}

void collection_append(Value value, Value item) {
    if (value.collection->count == value.collection->capacity) {
        size_t capacity = value.collection->capacity == 0 ? 4 : value.collection->capacity * 2;
        value.collection->items = realloc(value.collection->items, capacity * sizeof(Value));
        value.collection->capacity = capacity;
    }
    value.collection->items[value.collection->count++] = copy_value(item);
}

void dictionary_set(Value *value, const char *key, Value item) {
    DictEntry *entry = value->dictionary;
    while (entry && strcmp(entry->key, key) != 0) entry = entry->next;
    if (!entry) {
        entry = calloc(1, sizeof(*entry)); entry->key = duplicate_text(key);
        entry->value = malloc(sizeof(Value)); entry->next = value->dictionary;
        value->dictionary = entry;
    } else free_value(entry->value);
    *entry->value = copy_value(item);
}

Value *dictionary_get(Value value, const char *key) {
    for (DictEntry *entry = value.dictionary; entry; entry = entry->next)
        if (strcmp(entry->key, key) == 0) return entry->value;
    return NULL;
}

int dictionary_has(Value value, const char *key) { return dictionary_get(value, key) != NULL; }