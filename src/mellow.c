
#include <stdio.h>
#include <stdlib.h>


#include "lexer.h"
#include "utils/file.h"


int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("Usage: ./mellow <file.mll>\n");
        return 1;
    }

    char *source = read_file(argv[1]);
    if (source == NULL) 
        return 1;

    printf("%s\n", source);
    free(source);
    return 0;

}

