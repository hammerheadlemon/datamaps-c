#include "reader.h"

/* -- Some SQL strings ----------------------------- */

const char *dm_sql_str_create_table_datamap = "DROP TABLE IF EXISTS datamap;"
                                              "CREATE TABLE datamap(id INTEGER PRIMARY KEY, name TEXT, date_created TEXT);";
const char *dm_sql_str_create_table_datamapline = "DROP TABLE IF EXISTS datamap_line;"
                                                  "CREATE TABLE datamap_line("
                                                  "id INTEGER PRIMARY KEY,"
                                                  "dm_id INTEGER,"
                                                  "key TEXT NOT NULL,"
                                                  "sheet TEXT NOT NULL,"
                                                  "cellref TEXT,"
                                                  "FOREIGN KEY (dm_id)"
                                                  "   REFERENCES datamap(id)"
                                                  "   ON DELETE CASCADE"
                                                  ");";

/* -- HELPER FUNCS & CALLBACKS ----------------------------- */

// Helper function which returns a sqlite3 error and cleans up
extern void dm_sql_check_error(int rc, sqlite3 *db)
{
    if (rc != SQLITE_OK) {
        const char *err = sqlite3_errmsg(db);
        if(strncmp(err, "no such table", 14)) {
            fprintf(stderr, "Unable to set up database. Please try running with --initial option.");
        } else {
            fprintf(stderr, "Error #%d: %s\n", rc, err);
        }
        sqlite3_close(db);
        exit(rc);
    }
}

static int get_fields(char *line, size_t len)
{
        /* Let's work out how many fields we've got from the first line */
        int fieldcount = 0;
        for (int i = 0; i < len; i++) {
            if (line[i] == ',')
                fieldcount++;
        }
        return fieldcount;
}

extern int dm_exec_sql_stmt(const char *stmt, sqlite3 *db)
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

// This callback can be used as the third param in the sqlite3_exec() function
// to get the output result of that call in a_param. It gets the first element
// of the array and copies it in. Be careful with strings here.
static int exec_callback(void *a_param, int argc, char **argv, char **column) {
    strcpy(a_param, argv[0]);
    for (int i=0; i<argc; i++)
        printf("%s,\t", argv[i]);
    printf("\n");
    return 0;
}


/* -- MAIN FUNCTIONS ----------------------------- */

// Read the datamap file and populate a Datamapline struct with the data
extern int populate_datamapLine(char *line, Datamapline *dml)
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


/* Import a datamap file into the database */
/* dm_name is a name a user can add - CHECK THIS */
/* dm_overwrite flag indicates if we want to create a new table or not */
extern int dm_import_dm(char *dm_path, char *dm_name, int dm_overwrite)
{
    sqlite3 *db;

    // returns a return code
    int rc = sqlite3_open("test.db", &db);
    // handle error if this fails - we use this all over the place
    dm_sql_check_error(rc, db);

    rc = dm_exec_sql_stmt("PRAGMA foreign_keys = ON;",  db); // we have to do this every call

    // SQL to create the tables
    if(dm_overwrite) {
        fprintf(stdout, "Creating new tables in database.\n");
        rc = dm_exec_sql_stmt(dm_sql_str_create_table_datamap, db);
        rc = dm_exec_sql_stmt(dm_sql_str_create_table_datamapline, db);
    }

    // prep datamap create sql
    sqlite3_stmt *dm_create_stmt;
    const char *dm_sql = "INSERT INTO datamap VALUES (?,?,?)";
    rc = sqlite3_prepare_v2(db, dm_sql, -1, &dm_create_stmt, NULL);
    dm_sql_check_error(rc, db);

    // prep date sql to be used in datamap entry
    const char *date_sql = "SELECT datetime('now', 'localtime')";
    char date_str[23]; // the output of SELECT datetime() is 23 chars
    rc = sqlite3_exec(db, date_sql, exec_callback, &date_str, NULL);
    dm_sql_check_error(rc, db);

    // bind params
    sqlite3_bind_text(dm_create_stmt, 2, dm_name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(dm_create_stmt, 3, date_str, -1, SQLITE_TRANSIENT);

    // now execute the command to create the datamap entry
    rc = sqlite3_step(dm_create_stmt);

    int last_id = sqlite3_last_insert_rowid(db);

    /* Let's get on with opening the file and reading the stuff in. */ 
    FILE *stream = fopen(dm_path, "r");
    
    int lineno = 1;
    char line[1024];
    int expected_fields = 0;
    int fields;

    sqlite3_stmt *compiled_statement;

    const char *insert_sql = "INSERT INTO datamap_line VALUES(?,?,?,?,?);";
    
    char *err_msg;
    sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &err_msg);

    while (fgets(line, 1024, stream))
    {
        char tmp[1024];
        strcpy(tmp, line);

        if (lineno == 1) {
            expected_fields = get_fields(line, strlen(tmp));
            // we want to increment lineno here but we delay this
            // until after we have called get_fields()
        } 
        
        fields = get_fields(line, strlen(tmp));
        if (fields != expected_fields) {
            printf("Incorrect fields in line %s", tmp);
            printf("We do not allow narrative-style keys.\n");     
            continue;
        }
        
        // We use lineno for two purposes: to get header row
        // using get_fields() and for skipping the header row
        // when we import into database.
        if (lineno == 1) {
            lineno++;
            continue;
        }

        /* We pass in a shell of a struct to be populatum */
        Datamapline *dml = malloc(sizeof(Datamapline));
        if (populate_datamapLine(line, dml) == 1) {
            printf("Something went very wrong\n");
            return 1;
        }
    
        rc = sqlite3_prepare_v2(db, insert_sql, -1, &compiled_statement, NULL);
        dm_sql_check_error(rc, db);
        
        sqlite3_bind_int64(compiled_statement, 2, last_id);
        sqlite3_bind_text(compiled_statement, 3, dml->key, strlen(dml->key),SQLITE_TRANSIENT);
        sqlite3_bind_text(compiled_statement, 4, dml->sheet, strlen(dml->sheet),SQLITE_TRANSIENT);
        sqlite3_bind_text(compiled_statement, 5, dml->cellref, strlen(dml->cellref),SQLITE_TRANSIENT);

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
