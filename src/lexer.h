
#ifndef LEXER_H
#define LEXER_H

typedef enum {
    TOKEN_PRINT,
    TOKEN_OPEN,
    TOKEN_CLOSE,
    TOKEN_TEXT
} TokenType;


typedef struct {
    TokenType type;
    char *value;
} Token;

void lexer_run(char *source);


#endif
