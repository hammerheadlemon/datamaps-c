#include <stdio.h>
#include <unistd.h>
#include <getopt.h>

int main(int argc, char *argv[])
{
    char *cchoice = NULL;
    char *dchoice = NULL;
    printf("Welcome to Datamaps written in C\n\n");
    int option_index;

    opterr = 1;

    while ((option_index = getopt(argc, argv, ":c:d:")) != -1)
        switch (option_index) {
            case 'c':
                cchoice = optarg;
                printf("You get %s as your c option\n", cchoice);
                break;
            case 'd':
                dchoice = optarg;
                printf("You get %s as your d option\n", dchoice);
                break;
            case ':':
                printf("You want to run %s\n", optopt);
                break;
            default:
                printf("out\n");
        }

    return 0;
}
