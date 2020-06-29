#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

/* -- Datamap stuff ------------------------------------ */

#define EXCEL_MAX_SHEETNAME 256

// Data extracted from a datamap goes in here
typedef struct Datamapline {
    char *key;
    char sheet[EXCEL_MAX_SHEETNAME];
    char cellref[5];
} Datamapline;

extern int dm_import_dm(); // Import a datamap file into the database
extern int dm_populate_datamapLine(char *line, Datamapline *dml);


/* -- sqlite3 stuff ------------------------------------ */

extern void dm_sql_check_error(int rc, sqlite3 *db); // Helper function which returns a sqlite3 error and cleans up
extern int dm_exec_sql_stmt(const char *stmt, sqlite3 *db); // call a SQL statement in sqlite3

/* -- end of sqlite3 stuff ----------------------------- */
