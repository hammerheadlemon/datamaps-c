#include <stdio.h>
#include <error.h>
#include <stdlib.h>
#include <string.h>
#include <argp.h>
#include "reader.h"

//https://www.gnu.org/software/libc/manual/html_node/Argp-Example-4.html#Argp-Example-4

const char *argp_program_version = "datamaps 2.0";
const char *argp_program_bug_address = "datamaps@twentyfoursoftware.com";

// Program documentation
static char doc[] = "datamaps -- extract data from spreadsheets using key values stored in CSV files! That is it.";

// A description of the arguments we accept
static char args_doc[] = "datamap|import";

// Keys for options without short options
#define OPT_ABORT 1  // --abort

// https://stackoverflow.com/questions/47727755/gnu-argp-how-to-parse-option-with-only-long-name
// We use enums for setting long-only options here. The first one is set to
// 0x100 (256) - anything following doesn't need to be declared - they will go
// up to 0x101 I think and so on. You can start the next block at 0x200, etc.
enum datamap_options {
    DM_IMPORT = 0x100, // setting this to 0x100 (256) because non-ASCII characters ignored but we can still switch on it below
    DM_NAME,
    DM_OVERWRITE, // we want to start again with this datamap (DROP TABLE first)
    DM_INITIAL, // this has the same effect as DM_OVERWRITE in that it creates the db tables for the first time
    DM_IMPORT_SPREADSHEETS, // we import spreadsheets!
};

//The options we understand
static struct argp_option options[] = {
    { 0,0,0,0, "Global options:" },
    {"verbose", 'v', 0, 0, "Produce verbose output.", 1},
    {"quiet", 'q', 0, 0, "Don't produce any output.", 1},
    {"silent", 's', 0, OPTION_ALIAS},

    { 0,0,0,0, "Datamap options: (when calling 'datamaps datamap')." },
    {"import", DM_IMPORT, "PATH", 0, "PATH to datamap file to import."},
    {"name", DM_NAME, "NAME", 0, "The name you want to give to the imported datamap."},
    {"overwrite", DM_OVERWRITE, 0, 0, "Start fresh with this datamap (erases existing datamap data)."},
    {"initial", DM_INITIAL, 0, 0, "This option must be used where no datamap table yet exists."},

    { 0,0,0,0, "Relating to importing spreadsheets" },
    {"spreadsheet", DM_IMPORT_SPREADSHEETS, "PATH", 0, "PATH to spreadsheet to import."},

    { 0,0,0,0, "The following options should be grouped together:" },
    {"output", 'o', "FILE", 0, "Output to FILE instead of standard output."},
    {"repeat", 'r', "COUNT", OPTION_ARG_OPTIONAL, "Repeat the output COUNT (default 10) times."},
    {"abort", OPT_ABORT, 0, 0, "Abort before showing any output."},
    {0}
};

// Used by main to communicate with parse_opt.
struct arguments
{
    char *operation;
    char **strings;
    int silent, verbose;
    char *output_file;
    char *datamap_path;
    char *spreadsheet_path;
    char *dm_name;
    int dm_overwrite;
    int repeat;
    int abort;
};

// Parse a single option
static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
    // Get the input argument for arg_parse, which we
    // know is a pointer to our arguments structure.
    struct arguments *arguments = state->input;

    switch (key)
    {
        case 'q': case 's':
            arguments->silent = 1;
            break;
        case 'v':
            arguments->verbose = 1;
        case 'o':
            arguments->output_file = arg;
            break;
        case DM_IMPORT:
            arguments->datamap_path = arg;
            break;
        case DM_NAME:
            arguments->dm_name = arg;
            break;
        case DM_OVERWRITE: case DM_INITIAL:
            arguments->dm_overwrite = 1;
            break;
        case DM_IMPORT_SPREADSHEETS:
            arguments->spreadsheet_path = arg;
            break;
        case 'r':
            arguments->repeat = arg ? atoi (arg) : 10;
            break;
        case OPT_ABORT:
            arguments->abort = 1;
            break;
        case ARGP_KEY_NO_ARGS:
            argp_usage(state);
        case ARGP_KEY_ARG:
            // Here we know that state->arg_num == 0, since we force
            // argument parsing to end before any more arguments can get here.
            arguments->operation = arg;

            // Now we consume all the rest of the arguments.
            // state->next is the index in state->argv of the next
            // argument to be parsed, which is the first string
            // we're interested in, so we can just use
            // &state->argv[state->next] as the value for arguments-> strings
            //
            // In addition, by setting state->next to the end of the arguments,
            // we can force argp to stop parsing here and return.
            arguments->strings = &state->argv[state->next];
            state->next = state->argc;
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc};

int main(int argc, char *argv[])
{
    int i, j;
    struct arguments arguments;

    // Default values
    arguments.silent = 0;
    arguments.verbose = 0;
    arguments.output_file = "-";
    arguments.repeat = 1;
    arguments.abort = 0;
    arguments.datamap_path = "";
    arguments.spreadsheet_path = "";
    arguments.dm_name = "New datamap";
    arguments.dm_overwrite = 0;

    // Parse our arguments; every option seen by parse_opt will be
    // reflected in arguments.
    argp_parse(&argp, argc, argv, 0, 0, &arguments);
    
    // This should not be called here but it offers a nice message
    /* argp_help(&argp, stderr, ARGP_HELP_SEE, "datamaps"); */
    
    if(arguments.abort)
        error(10, 0, "ABORTED");

    if (strcmp("datamap", arguments.operation) == 0) {
        printf("Importing datamap file at %s\n", arguments.datamap_path);
        printf("DATAMAP_OVERWRITE = %d\n", arguments.dm_overwrite);
        dm_import_dm(arguments.datamap_path, arguments.dm_name, arguments.dm_overwrite);
    }
    else if (strcmp("import", arguments.operation) == 0) {
        printf("We are going to call an import() func here.\n");
        if(strcmp("", arguments.spreadsheet_path) == 0) {
            fprintf(stderr, "You probably need to use the --spreadsheet option here.\n");
        }
        read_spreadsheet(arguments.spreadsheet_path);
    }

    for (i = 0; i < arguments.repeat; ++i) {
        printf("ARG1 = %s\n", arguments.operation);
        printf("STRINGS = ");
        for (j = 0; arguments.strings[j]; ++j) {
           printf(j==0 ? "%s" : ", %s", arguments.strings[j]); 
        }
        printf("\n");
        printf("DATAMAP_PATH = %s\n", arguments.datamap_path);
        printf("OUTPUT_FILE = %s\nVERBOSE = %s\nSILENT = %s\n",
                arguments.output_file,
                arguments.verbose ? "yes" : "no",
                arguments.silent ? "yes" : "no");
    }

    exit(0);
}
