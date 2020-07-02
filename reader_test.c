#include <glib.h>
#include "reader.h"

typedef struct {
    char *dm_file;
    Datamapline dml;
} dm_fixture;

void dm_setup(dm_fixture *df, gconstpointer test_data) {
    Datamapline test_dml = {"Test Key", "Test Sheet", "A1"};
    df->dm_file = "tits";
    df->dml = test_dml;
    printf("In dm_setup - data is %d\n", *df->dm_file);
    printf("Test DML key is %s\n", df->dml.key);
}

void test_parse_dm(dm_fixture *df, gconstpointer ignored){
    printf("In test_parse_dm\n");
}

void dm_teardown(dm_fixture *df, gconstpointer test_data) {
    printf("In dm_teardown\n");
}

int main(int argc, char **argv) {
    g_test_init(&argc, &argv, NULL);
    g_test_add("/set1/new test", dm_fixture, NULL, dm_setup, test_parse_dm, dm_teardown);
    return g_test_run();
}
