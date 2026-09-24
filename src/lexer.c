#include "lexer.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char *start;
    const char *current;
    int line;
    int column;
} Scanner;

static char *copy_text(const char *start, size_t length) {
    char *text = malloc(length + 1);
    if (text == NULL) return NULL;
    memcpy(text, start, length);
    text[length] = '\0';
    return text;
}

static void push_token(TokenList *tokens, TokenType type, const char *start,
                       size_t length, int line, int column) {
    if (tokens->count == tokens->capacity) {
        size_t capacity = tokens->capacity == 0 ? 32 : tokens->capacity * 2;
        Token *items = realloc(tokens->items, capacity * sizeof(*items));
        if (items == NULL) return;
        tokens->items = items;
        tokens->capacity = capacity;
    }
    Token *token = &tokens->items[tokens->count++];
    token->type = type;
    token->lexeme = copy_text(start, length);
    token->line = line;
    token->column = column;
}

static TokenType keyword_type(const char *text) {
    static const struct { const char *name; TokenType type; } keywords[] = {
        {"true", TOKEN_TRUE}, {"false", TOKEN_FALSE}, {"null", TOKEN_NULL},
        {"let", TOKEN_LET}, {"const", TOKEN_CONST}, {"if", TOKEN_IF},
        {"else", TOKEN_ELSE}, {"while", TOKEN_WHILE}, {"for", TOKEN_FOR},
        {"loop", TOKEN_LOOP}, {"in", TOKEN_IN}, {"break", TOKEN_BREAK}, {"continue", TOKEN_CONTINUE},
        {"return", TOKEN_RETURN}, {"func", TOKEN_FUNC}, {"class", TOKEN_CLASS},
        {"public", TOKEN_PUBLIC}, {"private", TOKEN_PRIVATE}, {"import", TOKEN_IMPORT},
        {"try", TOKEN_TRY}, {"catch", TOKEN_CATCH}, {"raise", TOKEN_RAISE},
        {"fn", TOKEN_FN}, {"and", TOKEN_AND}, {"or", TOKEN_OR}, {"not", TOKEN_NOT},
        {"this", TOKEN_THIS}, {"super", TOKEN_SUPER}
    };
    for (size_t i = 0; i < sizeof(keywords) / sizeof(*keywords); i++)
        if (strcmp(text, keywords[i].name) == 0) return keywords[i].type;
    return TOKEN_IDENTIFIER;
}

static void scan_string(Scanner *scanner, TokenList *tokens, char quote) {
    const char *start = ++scanner->current;
    int line = scanner->line;
    int column = scanner->column++;
    while (*scanner->current != quote && *scanner->current != '\0') {
        if (*scanner->current == '\n') { scanner->line++; scanner->column = 1; }
        else scanner->column++;
        scanner->current++;
    }
    if (*scanner->current == '\0') {
        push_token(tokens, TOKEN_ERROR, start, (size_t)(scanner->current - start),
                   line, column);
        return;
    }
    push_token(tokens, quote == '"' ? TOKEN_STRING : TOKEN_CHAR, start,
               (size_t)(scanner->current - start), line, column);
    scanner->current++;
    scanner->column++;
}

TokenList lexer_scan(const char *source) {
    TokenList tokens = {0};
    Scanner scanner = {source, source, 1, 1};
    while (*scanner.current != '\0') {
        char c = *scanner.current;
        if (c == ' ' || c == '\t' || c == '\r') { scanner.current++; scanner.column++; continue; }
        if (c == '\n') { push_token(&tokens, TOKEN_NEWLINE, "", 0, scanner.line, scanner.column); scanner.current++; scanner.line++; scanner.column = 1; continue; }
        if (c == '/' && scanner.current[1] == '/') { while (*scanner.current && *scanner.current != '\n') { scanner.current++; scanner.column++; } continue; }
        if (c == '/' && scanner.current[1] == '*') {
            scanner.current += 2; scanner.column += 2;
            while (*scanner.current && !(scanner.current[0] == '*' && scanner.current[1] == '/')) {
                if (*scanner.current == '\n') { scanner.line++; scanner.column = 1; } else scanner.column++;
                scanner.current++;
            }
            if (*scanner.current) { scanner.current += 2; scanner.column += 2; }
            continue;
        }
        int line = scanner.line, column = scanner.column;
        if (isalpha((unsigned char)c) || c == '_') {
            const char *start = scanner.current;
            while (isalnum((unsigned char)*scanner.current) || *scanner.current == '_') { scanner.current++; scanner.column++; }
            char *text = copy_text(start, (size_t)(scanner.current - start));
            TokenType type = text ? keyword_type(text) : TOKEN_ERROR;
            free(text); push_token(&tokens, type, start, (size_t)(scanner.current - start), line, column); continue;
        }
        if (isdigit((unsigned char)c) || (c == '.' && isdigit((unsigned char)scanner.current[1]))) {
            const char *start = scanner.current; int dots = 0;
            while (isdigit((unsigned char)*scanner.current) || *scanner.current == '.') { if (*scanner.current == '.') dots++; scanner.current++; scanner.column++; }
            push_token(&tokens, dots > 1 ? TOKEN_ERROR : TOKEN_NUMBER, start, (size_t)(scanner.current - start), line, column); continue;
        }
        if (c == '"' || c == '\'') { scan_string(&scanner, &tokens, c); continue; }
        TokenType type = TOKEN_ERROR;
        switch (c) {
            case '(': type = TOKEN_LEFT_PAREN; break; case ')': type = TOKEN_RIGHT_PAREN; break;
            case '<': type = TOKEN_LEFT_ANGLE; break; case '>': type = TOKEN_RIGHT_ANGLE; break;
            case '[': type = TOKEN_LEFT_BRACKET; break; case ']': type = TOKEN_RIGHT_BRACKET; break;
            case '{': type = TOKEN_LEFT_BRACE; break; case '}': type = TOKEN_RIGHT_BRACE; break;
            case ',': type = TOKEN_COMMA; break; case ':': type = TOKEN_COLON; break;
            case '.': type = TOKEN_DOT; break; case '=': type = TOKEN_EQUAL; break;
            case '|': if (scanner.current[1] == '>') { type = TOKEN_PIPE; scanner.current++; scanner.column++; } break;
            case ';': type = TOKEN_SEMICOLON; break;
        }
        push_token(&tokens, type, scanner.current, 1, line, column);
        scanner.current++; scanner.column++;
    }
    push_token(&tokens, TOKEN_EOF, "", 0, scanner.line, scanner.column);
    return tokens;
}

void token_list_free(TokenList *tokens) {
    for (size_t i = 0; i < tokens->count; i++) free(tokens->items[i].lexeme);
    free(tokens->items); tokens->items = NULL; tokens->count = tokens->capacity = 0;
}

const char *token_type_name(TokenType type) {
    static const char *names[] = {"end of file", "error", "identifier", "number", "string", "char", "true", "false", "null", "let", "const", "if", "else", "while", "for", "in", "loop", "break", "continue", "return", "func", "class", "public", "private", "import", "try", "catch", "raise", "fn", "and", "or", "not", "this", "super", "newline", "(", ")", "<", ">", "[", "]", "{", "}", ",", ":", ".", "=", "|>", ";"};
    return type <= TOKEN_SEMICOLON ? names[type] : "unknown";
}

#include "lexer.h"
#include <stdio.h>
#include <string.h>


void lexer_run(char *source) {

    if (strncmp(source, "Print", 5) == 0) {
        printf("Found Print!\n");
    }
    

}


