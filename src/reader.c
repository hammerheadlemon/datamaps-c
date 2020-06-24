#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "reader.h"

typedef struct Datamapline {
    char *key;
    char *sheet;
    char *cellref;
} Datamapline;

static const Datamapline *createDatamapLine(char *line)
{
    struct Datamapline *dml;
    dml = malloc(sizeof(Datamapline));

    int count = 1;
    char *tok;
    for (tok = strtok(line, ",");
            tok !=NULL;
            tok = strtok(NULL, ","))
    {
        if (count == 1) {
            dml->key = tok;
            count++;
        } else if (count == 2) {
            dml->sheet = tok;
            count++;
        } else if (count == 3) {
            dml->cellref = tok;
            count++;
        }
    }
    return dml;
}

int import_csv()
{
    FILE *stream = fopen("/home/lemon/Documents/datamaps/input/datamap.csv", "r");
    
    char line[1024];
    while (fgets(line, 1024, stream))
    {
        char *tmp = strdup(line);
        const Datamapline *dml = createDatamapLine(tmp);
        printf("%-10s %s\n", "Key:",  dml->key);
        printf("%-10s %s\n", "Sheet:",  dml->sheet);
        printf("%-10s %s\n\n", "Cellref:", dml->cellref);

        free(tmp);
    }
    return 0;
}
