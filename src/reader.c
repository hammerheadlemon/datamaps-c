#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "reader.h"
#include <sqlite3.h>

/*
 * How do we want to store the Datamapline structs?
 */

typedef struct Datamapline {
    char *key;
    char sheet[EXCEL_MAX_SHEETNAME];
    char cellref[5];
} Datamapline;

void check_error(int result_code, sqlite3 *db)
{
    if (result_code != SQLITE_OK) {
        fprintf(stderr, "Error #%d: %s\n", result_code, sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(result_code);
    }
}


/* This fails badly if it reads in a line that has more than three fields! */
/* TODO: fix it */
// Having said that, do we want to develop a generic CSV reader? Probably not
// at this stage, although we could use it elsewhere.
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

int sql_stmt(const char *stmt, sqlite3 *db)
{
    char *err_msg;
    int ret;

    ret = sqlite3_exec(db, stmt, 0, 0, &err_msg);

    if (ret != SQLITE_OK) {
        fprintf(stderr, "Error in statement: %s [%s].\n", stmt, err_msg);
        return 1;
    }
    return SQLITE_OK;
}

// Import a datamap file into the database
int dm_import(char *dm_path)
{
    sqlite3 *db;

    // returns a return code
    int rc = sqlite3_open("test.db", &db);
    // handle error if this fails
    check_error(rc, db);

    // SQL to create the table
    rc = sql_stmt("DROP TABLE IF EXISTS datamap;"
             "CREATE TABLE datamap(id INTEGER PRIMARY KEY, key TEXT, sheet TEXT, cellref TEXT);"
             ,db);
    check_error(rc, db);


    FILE *stream = fopen(dm_path, "r");
    
    int lineno = 1;
    char line[1024];
    int expected_fields = 0;
    int fields;

    sqlite3_stmt *compiled_statement;

    const char *insert_sql = "INSERT INTO datamap VALUES(?,?,?,?);";
    
    char *err_msg;
    sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &err_msg);

    while (fgets(line, 1024, stream))
    {
        char tmp[1024];
        strcpy(tmp, line);

        if (lineno == 1) {
            expected_fields = getFields(line, strlen(tmp));
            // we want to increment lineno here but we delay this
            // until after we have called getFields()
        } 
        
        fields = getFields(line, strlen(tmp));
        if (fields != expected_fields) {
            printf("Incorrect fields in line %s", tmp);
            printf("We do not allow narrative-style keys.\n");     
            continue;
        }
        
        // We use lineno for two purposes: to get header row
        // using getFields() and for skipping the header row
        // when we import into database.
        if (lineno == 1) {
            lineno++;
            continue;
        }

        /* We pass in a shell of a struct to be populatum */
        Datamapline *dml = malloc(sizeof(Datamapline));
        if (populateDatamapLine(line, dml) == 1) {
            printf("Something went very wrong\n");
            return 1;
        }
    
        rc = sqlite3_prepare_v2(db, insert_sql, -1, &compiled_statement, NULL);
        check_error(rc, db);
        
        sqlite3_bind_text(compiled_statement, 2, dml->key, strlen(dml->key),SQLITE_TRANSIENT);
        sqlite3_bind_text(compiled_statement, 3, dml->sheet, strlen(dml->sheet),SQLITE_TRANSIENT);
        sqlite3_bind_text(compiled_statement, 4, dml->cellref, strlen(dml->cellref),SQLITE_TRANSIENT);

        rc = sqlite3_step(compiled_statement);

        if(rc != SQLITE_DONE) {
            fprintf(stderr, "Error #%d: %s\n", rc, sqlite3_errmsg(db));
            sqlite3_close(db);
            return rc;
        }

        /* sqlite3_finalize(compiled_statement); */
        /* sqlite3_clear_bindings(compiled_statement); */
        /* sqlite3_reset(compiled_statement); */

        /* printf("%-10s %s\n", "Key:",  dml->key); */
        /* printf("%-10s %s\n", "Sheet:",  dml->sheet); */
        /* printf("%-10s %s\n", "Cellref:", dml->cellref); */

        free(dml);
        sqlite3_clear_bindings(compiled_statement);
        sqlite3_reset(compiled_statement);
        sqlite3_finalize(compiled_statement);
    }
        sqlite3_exec(db, "END TRANSACTION", NULL, NULL, &err_msg);
        sqlite3_close(db);
        return 0;
}
