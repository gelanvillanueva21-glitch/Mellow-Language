

#include "file.h"
#include <stdio.h>
#include <stdlib.h>


char *read_file(char *filename) {

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Could not open file.\n");
        return NULL;
    }
    
    char *source = malloc(1000);

    if (source == NULL) {
        printf("Could not allocate memory.\n");
        fclose(file);
        return NULL;
    }

    int index = 0;
    int character;

    while ((character = fgetc(file)) != EOF) {
        source[index] = character;
        index++;
    }

    source[index] = '\0';
    fclose(file);
    return source;

}

