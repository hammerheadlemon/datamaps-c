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
static char args_doc[] = "import|export";

// Keys for options without short options
#define OPT_ABORT 1  // --abort

//The options we understand
static struct argp_option options[] = {
    {"verbose", 'v', 0, 0, "Produce verbose output"},
    {"quiet", 'q', 0, 0, "Don't produce any output"},
    {"silent", 's', 0, OPTION_ALIAS},
    {"output", 'o', "FILE", 0, "Output to FILE instead of standard output"},
    {"datamap", 'd', "PATH", 0, "PATH to datamap file to use instead of default"},
    { 0,0,0,0, "The following options should be grouped together:" },
    {"repeat", 'r', "COUNT", OPTION_ARG_OPTIONAL, "Repeat the output COUNT (default 10) times"},
    {"abort", OPT_ABORT, 0, 0, "Abort before showing any output"},
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
        case 'd':
            arguments->datamap_path = arg;
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

    // Parse our arguments; every option seen by parse_opt will be
    // reflected in arguments.
    argp_parse(&argp, argc, argv, 0, 0, &arguments);
    
    // This should not be called here but it offers a nice message
    /* argp_help(&argp, stderr, ARGP_HELP_SEE, "datamaps"); */
    
    if(arguments.abort)
        error(10, 0, "ABORTED");

    if (strcmp("import", arguments.operation) == 0) {
        printf("We are going to call an import() func here.\n");
        if (strcmp("", arguments.datamap_path) == 0) {
            printf("Please use -d to define a path to your datamap file.\n");
        } else {
            import_csv(arguments.datamap_path);
        }
    }
    else if (strcmp("export", arguments.operation) == 0)
        printf("We are going to call an export() func here.\n");

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
