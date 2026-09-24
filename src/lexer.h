
#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>
typedef enum {
    TOKEN_EOF,
    TOKEN_ERROR,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_CHAR,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_NULL,
    TOKEN_LET,
    TOKEN_CONST,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_FOR,
    TOKEN_IN,
    TOKEN_LOOP,
    TOKEN_BREAK,
    TOKEN_CONTINUE,
    TOKEN_RETURN,
    TOKEN_FUNC,
    TOKEN_CLASS,
    TOKEN_PUBLIC,
    TOKEN_PRIVATE,
    TOKEN_IMPORT,
    TOKEN_TRY,
    TOKEN_CATCH,
    TOKEN_RAISE,
    TOKEN_FN,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_NOT,
    TOKEN_THIS,
    TOKEN_SUPER,
    TOKEN_NEWLINE,
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_ANGLE,
    TOKEN_RIGHT_ANGLE,
    TOKEN_LEFT_BRACKET,
    TOKEN_RIGHT_BRACKET,
    TOKEN_LEFT_BRACE,
    TOKEN_RIGHT_BRACE,
    TOKEN_COMMA,
    TOKEN_COLON,
    TOKEN_DOT,
    TOKEN_EQUAL,
    TOKEN_PIPE,
    TOKEN_SEMICOLON
} TokenType;

typedef struct {
    TokenType type;
    char *lexeme;
    int line;
    int column;
} Token;

typedef struct {
    Token *items;
    size_t count;
    size_t capacity;
} TokenList;

TokenList lexer_scan(const char *source);
void token_list_free(TokenList *tokens);
const char *token_type_name(TokenType type);


#endif
