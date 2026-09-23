

#include "file.h"
#include <stdio.h>
#include <stdlib.h>


char *read_file(char *filename) {

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Could not open file.\n");
        return NULL;
    }
    
    size_t capacity = 4096;
    char *source = malloc(capacity);

    if (source == NULL) {
        printf("Could not allocate memory.\n");
        fclose(file);
        return NULL;
    }

    size_t index = 0;
    int character;

    while ((character = fgetc(file)) != EOF) {
        if (index + 1 >= capacity) {
            capacity *= 2;
            char *expanded = realloc(source, capacity);
            if (expanded == NULL) {
                printf("Could not allocate memory.\n");
                free(source);
                fclose(file);
                return NULL;
            }
            source = expanded;
        }
        source[index] = character;
        index++;
    }

    source[index] = '\0';
    fclose(file);
    return source;

}

