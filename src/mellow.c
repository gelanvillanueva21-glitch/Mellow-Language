
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "runtime.h"
#include "utils/file.h"

static void print_tokens(TokenList *tokens) {
    for (size_t i = 0; i < tokens->count; i++) {
        Token *token = &tokens->items[i];
        printf("%d:%d %-12s %s\n", token->line, token->column,
               token_type_name(token->type), token->lexeme);
    }
}

static int run_repl(void) {
    char line[4096];
    printf("Mellow REPL. Type exit to quit.\n");
    while (printf("> "), fflush(stdout), fgets(line, sizeof(line), stdin)) {
        if (strcmp(line, "exit\n") == 0 || strcmp(line, "exit") == 0) break;
        TokenList tokens = lexer_scan(line);
        runtime_run(&tokens);
        token_list_free(&tokens);
    }
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc == 2 && strcmp(argv[1], "--repl") == 0) return run_repl();
    int token_mode = argc > 1 && strcmp(argv[1], "--tokens") == 0;
    int file_index = token_mode ? 2 : 1;

    if (argc <= file_index) {
        printf("Usage: ./mellow [--tokens] <file.mll>\n");
        return 1;
    }

    char *source = read_file(argv[file_index]);
    if (source == NULL) 
        return 1;

    TokenList tokens = lexer_scan(source);
    if (token_mode) print_tokens(&tokens);
    else if (runtime_run(&tokens) != 0) { token_list_free(&tokens); free(source); return 1; }
    token_list_free(&tokens);
    free(source);
    return 0;

}

