#include <glib.h>
#include "reader.h"

typedef struct {
    char *dm_file;
} dm_fixture;

void dm_setup(dm_fixture *df, gconstpointer test_data) {
    
}

int main(int argc, char **argv) {
    g_test_init(&argc, &argv, NULL);
}
