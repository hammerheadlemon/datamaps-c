#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "reader.h"

typedef struct Datamapline {
    char *key;
    char sheet[EXCEL_MAX_SHEETNAME];
    char cellref[5];
} Datamapline;

/* This fails badly if it reads in a line that has more than three fields! */
/* TODO: fix it */
static int populateDatamapLine(char *line, Datamapline *dml)
{

    int count = 1;
    char *tok;
    /* we don't want strtok to do more than commas */
    for (tok = strtok(line, ",");
            tok !=NULL;
            tok = strtok(NULL, ","))
    {
        if (count == 1) {
            dml->key = tok;
            count++;
        } else if (count == 2) {
            strcpy(dml->sheet, tok);
            count++;
        } else if (count == 3) {
            strcpy(dml->cellref, tok);
            count++;
        }
    }
    return 0;
}

static int getFields(char *line, size_t len)
{
        /* Let's work out how many fields we've got from the first line */
        int fieldcount = 0;
        for (int i = 0; i < len; i++) {
            if (line[i] == ',')
                fieldcount++;
        }
        return fieldcount;
}

int import_csv(char *dm_path)
{
    FILE *stream = fopen(dm_path, "r");
    
    int lineno = 1;
    char line[1024];
    int expected_fields = 0;
    int fields;

    while (fgets(line, 1024, stream))
    {
        char tmp[1024];
        strcpy(tmp, line);

        if (lineno == 1) {
            expected_fields = getFields(line, strlen(tmp));
            lineno++;
        } 
        
        fields = getFields(line, strlen(tmp));
        if (fields != expected_fields) {
            printf("Incorrect fields in line %s", tmp);
            printf("We do not allow narrative-style keys.\n");     
            continue;
        }
        
        /* We pass in a shell of a struct to be populatum */
        Datamapline *dml = malloc(sizeof(Datamapline));
        if (populateDatamapLine(line, dml) == 1) {
            printf("Something went very wrong\n");
            return 1;
        }

        printf("%-10s %s\n", "Key:",  dml->key);
        printf("%-10s %s\n", "Sheet:",  dml->sheet);
        printf("%-10s %s\n", "Cellref:", dml->cellref);

        free(dml);
    }
    return 0;
}
