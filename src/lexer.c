
#include "lexer.h"
#include <stdio.h>
#include <string.h>


void lexer_run(char *source) {

    if (strncmp(source, "Print", 5) == 0) {
        printf("Found Print!\n");
    }
    

}


